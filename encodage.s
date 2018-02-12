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
    global _find_word

_file_size:
	enter 24, 0
; lseek to start of file
	xor rax, rax
	mov rax, 8
	mov rsi, 0
	mov rdx, 0
	syscall
; lseek to end of file
	mov rax, 8
	mov rsi, 0
	mov rdx, 2
	syscall
; store the return value, it's the offset of EOF. So it's the file size
; lseek to start of file again
	mov QWORD [rsp], rax
	mov rax, 8
	mov rsi, 0
	mov rdx, 0
	syscall
	mov rax, QWORD [rsp]
	leave
	ret

_start:
    enter 80, 0

;; Check if argc == 2
    cmp QWORD [rsp + 40], 2
    jne _end

;; Open file
    mov rax, 1
    mov rdi, QWORD [rsp + 56]
    mov rsi, 2
    syscall
    cmp rax, 0
    jle _end
    mov QWORD [rsp], rax

;; Call file size
    mov rdi, QWORD [rsp]
    call _file_size
    mov QWORD [rsp + 8], rax

;; First mmap, mmap the open file
	mov rax, 9
	mov rdi, 0
	mov rsi, QWORD [rsp + 8]
	mov rdx, 3
	mov r10, 2
	mov r8, QWORD [rsp]
	mov r9, 0
	syscall
    jle _close_file
    mov QWORD [rsp + 16], rax

;; Second mmap, of the size of the file, for the dictionnary
	mov rax, 9
	mov rdi, 0
	mov rsi, QWORD [rsp + 8]
	mov rdx, 3
	mov r10, 34
	mov r8, -1
	mov r9, 0
	syscall
	cmp rax, 0
	jle _close_file
    mov QWORD [rsp + 24], rax

;; Third mmap, of the size of the file, for the compressed datas
	mov rax, 9
	mov rdi, 0
	mov rsi, QWORD [rsp + 8]
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
    .loop
    mov rdi, QWORD [rsp + 16]
    mov rsi, QWORD [rsp + 24]
    mov rdx, QWORD [rsp + 32]
    add rdi, QWORD [rsp + 40]
    add rdx, QWORD [rsp + 48]
    call _find_word
    add QWORD [rsp + 40], rax
    add QWORD [rsp + 48], 4
    mov rax, QWORD [rsp + 40]
    cmp rax, QWORD [rsp + 8]
    jl _create_structs.loop

;;; Open our new file
;    mov rax, 1
;    lea rdi, [rel _new_file.string]
;    mov rsi, 578
;    mov rdx, 493
;    syscall
;    cmp rax, 0
;    jle _end
;    mov QWORD [rsp + 8], rax
;; Just mmap the file to compress, travel it a first time,
;; to create the dictionnary in a new file, then
;; travel it again, and replace word according to the dictionnary.

;; Close file
_close_file:
    mov rax, 3
    mov rdi, QWORD [rsp]
    syscall

;; Finish
_end:
    leave
    mov rax, 60
    mov rdi, 0
    syscall

_find_word: ; int _find_word(void *addr_of_word, void *addr_dictionnary, void *addr_of_index_struct)
    enter 64, 0
    mov QWORD [rsp], rdi ; word addr
    mov QWORD [rsp + 8], rsi ; dictionnary addr
    mov QWORD [rsp + 16], rdx ; struct index addr

;; First, find a word. A word is delimited by a duplicate byte. So if we have:
;; "test"
;; the first word will be "tes", and the second one will be "t"
_define_word:
    .init
    mov QWORD [rsp + 24], 0 ; word size
    .loop
    add QWORD [rsp + 24], 1
    mov rsi, QWORD [rsp]
    add rsi, QWORD [rsp + 24]
    xor rax, rax
    mov al, BYTE [rsi]
    mov rcx, QWORD [rsp + 24]
    mov rdi, QWORD [rsp]
    repne scasb
    jne _define_word.loop

_update_words_dictionnary
    mov rdi, QWORD [rsp + 8]
    .loop
    add rdi, 2
    mov rax, QWORD [rsp + 24]
    cmp WORD [rdi], rax
    je _compare
    cmp WORD [rdi], 0
    je _add_in_dictionnary

_increment_reloop:
    xor rsi, rsi
    mov si, WORD [rdi]
    add rdi, rsi
    jmp _update_words_dictionnary.loop

_compare:
    add rdi, 2
    mov rsi, rdi
    mov rdi, QWORD [rsp]
    mov rcx, QWORD [rsp + 24]
    push rsi
    cld
    repe cmpsb
    pop rdi
    sub rdi, 2
    jne _increment_reloop
    jmp _update_index_struct

_add_in_dictionnary:
    mov rax, QWORD [rsp + 24]
    mov WORD [rdi], ax
    mov rsi, WORD [rdi - 2]
    inc rsi
    mov WORD [rdi + ax], rsi
    mov rsi, QWORD [rsp]
    add rdi, 2
    mov rcx, QWORD [rsp + 24]
    cld
    rep movsb

_update_index_struct:
    mov rsi, rdi
    sub rsi, 2
    mov si, WORD [rsi]
    mov rdi, QWORD [rsp + 16]
    mov WORD [rdi], 1
    mov WORD [rdi + 2], si

_ret_find_word:
    mov rax, QWORD [rsp + 24]
    leave
    ret
