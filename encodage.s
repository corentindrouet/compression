;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;   header:
;   dictionnary offset from start: 8 bytes
;   compressed datas offset from start: 8 bytes
;
;   dictionnary:
;   2 bytes (dword), word replacing data_word
;   2 bytes size of data_word
;   data_word
;
;   compressed datas:
;   2 bytes, multiplicator
;   2 bytes, word
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
section .data
_new_file:
    .string db './test', 0
    .len equ $ - _new_file.string

section .text
    global _start
    global _find_word ; int (void *addr_of_word, void *addr_dictionnary, void *addr_of_index_struct)
	global _write_all_on_file ; void (dictionnary addr, compressed datas addr, compressed datas size)

_file_size:
	enter 24, 0
	push rdi
; lseek to start of file
	xor rax, rax
	mov rax, 8
	mov rsi, 0
	mov rdx, 0
	mov rdi, QWORD [rsp]
	syscall
; lseek to end of file
	mov rax, 8
	mov rsi, 0
	mov rdx, 2
	mov rdi, QWORD [rsp]
	syscall
; store the return value, it's the offset of EOF. So it's the file size
; lseek to start of file again
	mov QWORD [rsp + 8], rax
	mov rdi, QWORD [rsp]
	mov rax, 8
	mov rsi, 0
	mov rdx, 0
	syscall
	mov rax, QWORD [rsp + 8]
	leave
	ret

_start:
    enter 80, 0

;; Check if argc == 2
_check_arg:
    cmp QWORD [rsp + 88], 2
    jne _end

;; Open file
_open_file:
    mov rax, 2
    mov rdi, QWORD [rsp + 104]
    mov rsi, 2
    syscall
    cmp rax, 0
    jle _end
    mov QWORD [rsp], rax

;; Call file size
_take_file_size:
    mov rdi, QWORD [rsp]
    call _file_size
    mov QWORD [rsp + 8], rax

;; First mmap, mmap the open file
_first_mmap:
	mov rax, 9
	mov rdi, 0
	mov rsi, QWORD [rsp + 8]
	mov rdx, 3
	mov r10, 2
	mov r8, QWORD [rsp]
	mov r9, 0
	syscall
	cmp rax, 0
    jle _close_file
    mov QWORD [rsp + 16], rax

;; Second mmap, of the size of the file, for the dictionnary
_second_mmap:
	mov rax, 9
	mov rdi, 0
	mov rsi, QWORD [rsp + 8]
	add rsi, 4096
	mov rdx, 3
	mov r10, 34
	mov r8, -1
	mov r9, 0
	syscall
	cmp rax, 0
	jle _close_file
    mov QWORD [rsp + 24], rax

;; Third mmap, of the size of the file, for the compressed datas
_third_mmap:
	mov rax, 9
	mov rdi, 0
	mov rsi, QWORD [rsp + 8]
	add rsi, 4096
	mov rdx, 3
	mov r10, 34
	mov r8, -1
	mov r9, 0
	syscall
	cmp rax, 0
	jle _close_file
    mov QWORD [rsp + 32], rax

;; Now we find words, and store our results
_create_structs:
    mov QWORD [rsp + 40], 0 ; word decalage
    mov QWORD [rsp + 48], 0 ; datas compressed decalage
;    mov QWORD [rsp + 56], 0 ; dictionnary size
    mov QWORD [rsp + 56], 0 ; compressed datas size
    .loop mov rdi, QWORD [rsp + 16] ; take file mmap
    mov rsi, QWORD [rsp + 24] ; take dictionnary mmap
    mov rdx, QWORD [rsp + 32] ; take datas mmap
    add rdi, QWORD [rsp + 40]
    add rdx, QWORD [rsp + 48]
    call _find_word
    add QWORD [rsp + 40], rax ; save our word size

;; Now we will incremente compressed datas offset only if our struct have been updated
	mov rdi, QWORD [rsp + 32]
	add rdi, QWORD [rsp + 48]
	cmp rdi, 0
	je _check_file_max_size
    add QWORD [rsp + 48], 4 ; if our current struct have been updated, we go to next struct
	add QWORD [rsp + 56], 4 ; update our compressed datas size

_check_file_max_size:
;; here we check if we didnt overflow our file size
    mov rax, QWORD [rsp + 40]
    cmp rax, QWORD [rsp + 8]
    jl _create_structs.loop
;; Just mmap the file to compress, travel it a first time,
;; to create the dictionnary in a new file, then
;; travel it again, and replace word according to the dictionnary.

_call_write:
	mov rdi, QWORD [rsp + 24]
	mov rsi, QWORD [rsp + 32]
	mov rdx, QWORD [rsp + 56]
	call _write_all_on_file

_munmap_first_file:
	mov rax, 11
	mov rdi, QWORD [rsp + 16]
	mov rsi, QWORD [rsp + 8]
	syscall

;; Close file
_close_file:
    mov rax, 3
    mov rdi, QWORD [rsp]
    syscall

_munmap_second_mmap:
	mov rax, 11
	mov rdi, QWORD [rsp + 24]
	mov rsi, QWORD [rsp + 8]
	add rsi, 4096
	syscall

_munmap_third_mmap:
	mov rax, 11
	mov rdi, QWORD [rsp + 32]
	mov rsi, QWORD [rsp + 8]
	add rsi, 4096
	syscall

;; Finish
_end:
    leave
    mov rax, 60
    mov rdi, 0
    syscall

_find_word: ; int _find_word(void *addr_of_word, void *addr_dictionnary, void *addr_of_index_struct)
    enter 64, 0 ; allocate 64 bytes on stack
    mov QWORD [rsp], rdi ; word addr
    mov QWORD [rsp + 8], rsi ; dictionnary addr
    mov QWORD [rsp + 16], rdx ; struct index addr

;; First, find a word. A word is delimited by a duplicate byte. So if we have:
;; "test"
;; the first word will be "tes", and the second one will be "t"
_define_word:
;; init the word size to 0
    .init mov QWORD [rsp + 24], 0 ; word size

;; loop to define word
;; First we take the current character + 1
    .loop add QWORD [rsp + 24], 1 ; increment word size, to don't compare first index with first index
    mov rsi, QWORD [rsp] ; take our string
    add rsi, QWORD [rsp + 24] ; increment our string addr with the size we already check

;; Now we check if our character is on our string
    xor rax, rax
    mov al, BYTE [rsi] ; take our character
    mov rcx, QWORD [rsp + 24] ; take our string size
    mov rdi, QWORD [rsp] ; take our string addr
    repne scasb ; try to find our character in our string
    jne _define_word.loop ; if we didn't find our character, our word isn't finished

_update_words_dictionnary:
;; we come here when we have our word
    mov rdi, QWORD [rsp + 8] ; take our dictionnary addr

;; now we will compare our word with other word of our dictionnary, to avoid duplicated word
    .loop add rdi, 2 ; the word size is at offset 2 of our word struct
    mov rax, QWORD [rsp + 24] ; take our word size
    cmp WORD [rdi], ax ; compare size
    je _compare ; if our word and dictionnary word have the same size, we compare them
;;if it's not the same size, we check if the size is null in the dictionnary word
    cmp WORD [rdi], 0
    je _add_in_dictionnary ;; if it's null, we are at the end of our dictionnary, so we need to add our word in

_increment_reloop:
;; here is the incrementation to go to the next struct of our word dictionnary
    xor rsi, rsi
    mov si, WORD [rdi] ; we take our current word size
    add rdi, rsi ; and we increment our current word struct addr
	add rdi, 2
    jmp _update_words_dictionnary.loop

_compare:
;; here we will compare our 2 words
    add rdi, 2 ; go to word first character of our dictionnary
    mov rsi, rdi ; mov our word addr on rsi
    mov rdi, QWORD [rsp] ; take our word addr
    mov rcx, QWORD [rsp + 24] ; size to compare
    push rsi ; save our dictionnary word addr
    cld
    repe cmpsb
    pop rdi
    sub rdi, 2
;; if the words didn't matched, we reloop on our dictionnary
    jne _increment_reloop
;; else if our words matched, we update our compressed datas list
    jmp _update_index_struct

_add_in_dictionnary:
;; here we add our word to the dictionnary
    mov rax, QWORD [rsp + 24] ; take our word size
    mov WORD [rdi], ax ; mov to the current dictionnary struct, our size

;; Our dictionnary is a mmap set to 0. So we concidere our first dictionnary word to have the index 0
;; And we create next dictionnary index with our current dictionnary index + 1.
;; we will update the next struct index
	xor rsi, rsi
    mov si, WORD [rdi - 2] ; take our current index number
    inc rsi ; incremente it
    mov WORD [rdi + rax + 2], si ; mov it at the end of our struct, for the next struct.

;; Here we mov our word on our dictionnary
    mov rsi, QWORD [rsp] ; take our word
    add rdi, 2 ; take our string addr
    mov rcx, QWORD [rsp + 24] ; number of characters to copy
	push rdi ; save our string addr
    cld
    rep movsb
	pop rdi ; recover our string addr
    sub rdi, 2

_update_index_struct:
;; here we will update our compressed datas with our word
	mov rsi, rdi
    mov si, WORD [rsi] ; we take our string size
    mov rdi, QWORD [rsp + 16] ; now we take our struct addr
;; our struct have a multiplicator, so we check if our previous struct is the same of our current struct
	cmp WORD [rdi - 2], si ; we compare dictionnary index of our previous struct with our current struct
	je _inc_last_index_multiplicator

;; if previous word struct isn't the same, we set a new struct of compressed datas:
    mov WORD [rdi], 1 ; our multiplicator
    mov WORD [rdi + 2], si ; our word struct index in dictionnary
	jmp _ret_find_word

_inc_last_index_multiplicator:
;; if previous struct and current struct are the same, we just increment previous struct multiplicator
	inc WORD [rdi - 4]

_ret_find_word:
;; we return the number of characters of our current word
    mov rax, QWORD [rsp + 24]
    leave
    ret

_write_all_on_file: ; void (dictionnary addr, compressed datas addr, compressed datas size)
;; We will write on file our header comporting our dictionnary offset, and our datas offset
;; So we will count the total dictionnary size, to take compressed datas offset.
	enter 56, 0
	mov QWORD [rsp], rdi ; dictionnary addr
	mov QWORD [rsp + 8], rsi ; compressed datas addr
	mov QWORD [rsp + 16], rdx ; compressed datas size
_count_dictionnary_size:
	.init mov QWORD [rsp + 24], 0
	.loop mov rdi, QWORD [rsp]
	add rdi, QWORD [rsp + 24]
	cmp WORD [rdi], 0
	je _end_dictionnary
	add QWORD [rsp + 24], 4
	xor rsi, rsi
	mov si, WORD [rdi + 2]
	add QWORD [rsp + 24], rsi
	jmp _count_dictionnary_size.loop

_end_dictionnary:
;; now we know all necessary information to right on file so, we write on file :)
;; Open our new file
    mov rax, 2
    lea rdi, [rel _new_file.string]
    mov rsi, 578
    mov rdx, 493
    syscall
    cmp rax, 0
    jle _end_write_file
    mov QWORD [rsp + 32], rax ; save fd

_write_header:
;; Write the first header
;; First we write dictionnary offset
	mov rax, 16
	push rax
	mov rax, 1
	mov rdi, QWORD [rsp + 40]
	mov rsi, rsp
	mov rdx, 8
	syscall
;; Second we write compressed datas offset
	mov rdi, QWORD [rsp + 40]
	mov rsi, rsp
	mov rax, QWORD [rsp + 32]
	add QWORD [rsp], rax
	mov rax, 1
	mov rdx, 8
	syscall
	pop rdi

_write_dictionnary:
;; Now we write the complete dictionnary on our file
	mov rax, 1
	mov rdi, QWORD [rsp + 32]
	mov rsi, QWORD [rsp]
	mov rdx, QWORD [rsp + 24]
	syscall

_write_compressed_datas:
;; Now we write our compressed datas table
	mov rax, 1
	mov rdi, QWORD [rsp + 32]
	mov rsi, QWORD [rsp + 8]
	mov rdx, QWORD [rsp + 16]
	syscall

_close_compressed_file:
    mov rax, 3
    mov rdi, QWORD [rsp + 32]
    syscall

_end_write_file:
;; And it's done
	leave
	ret
