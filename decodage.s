section .text
    global _start

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
    enter 40, 0

;; Check argc
_check_arg:
    cmp QWORD [rsp + 48], 3
    jne _end

_open_file:
    mov rax, 2
    mov rdi, QWORD [rsp + 64]
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
_mmap:
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

_call_decompress:
    mov rdi, QWORD [rsp + 16]
    mov rsi, QWORD [rsp + 8]
    mov rdx, QWORD [rsp + 72]
    call _decompress

_munmap:
    mov rax, 11
    mov rdi, QWORD [rsp + 16]
    mov rsi, QWORD [rsp + 8]
    syscall

_close_file:
    mov rax, 3
    mov rdi, QWORD [rsp]
    syscall

_end:
    leave
    mov rax, 60
    mov rdi, 0
    syscall

_decompress: ; void (file_mmap_addr, file_size, decompressed_file_name)
    enter 80, 0
    push rdx ; rsp + 16 : decompressed_file_name
    push rsi ; rsp + 8 : file_size
    push rdi ; rsp : file_mmap_addr

;; Now we take our dictionnary offset, and compressed datas offset
_take_offsets:
    mov rsi, QWORD [rsp]
    mov rdi, QWORD [rsi]
    mov QWORD [rsp + 24], rdi ; rsp + 24: dictionnary offset
    mov rdi, QWORD [rsi + 8]
    mov QWORD [rsp + 32], rdi ; rsp + 32: compressed datas offset
    
;; Now we open our new file
_open_new_file:
    mov rax, 2
    mov rdi, QWORD [rsp + 16]
    mov rsi, 578
    mov rdx, 493
    syscall
    cmp rax, 0
    jle _decompress_end
    mov QWORD [rsp + 40], rax ; save fd

;; Here we will browse our compressed datas, check the dictionnary according to the index,
;; and write the dictionnary word we find, directly on our file
_find_and_write:
    .loop mov rdi, QWORD [rsp + 32] ; take compressed datas offset
    add rdi, QWORD [rsp] ; add mmap base addr
    cmp WORD [rdi], 0 ; check if the multiplicator of the current compressed data is 0
    je _close_new_file ; if it's 0, we are at the end of our file
    
_find_word_on_dictionnary:
    .init mov rsi, QWORD [rsp + 24] ; take dictionnary offset
    add rsi, QWORD [rsp] ; add mmap base addr

    .loop xor r10, r10
    mov r10w, WORD [rsi] ; take current dictionnary word index
    cmp r10w, WORD [rdi + 2] ; compare it with our current compressed data index word
    je _write_word_on_file ; if it match, we write our dictionnary word

;; Go to next word n dictionnary
    mov r10w, WORD [rsi + 2] ; take word size on dictionnary
    add r10, 4 ; add 4 for the 4 bytes of dictionnary struct
    add rsi, r10 ; add the word size
    jmp _find_word_on_dictionnary.loop ; reloop

_write_word_on_file:
    .init xor r10, r10
    mov r10w, WORD [rdi] ; take our multiplicator
    mov QWORD [rsp + 48], r10 ; number of write for the current word
    mov QWORD [rsp + 56], rsi ; addr of current dictionnary

    .loop cmp QWORD [rsp + 48], 0 ; check if our multiplicator is 0
    jle _increment_compressed_datas_offset ; if so, we ended write our words

;; write
    mov rax, 1
    mov rdi, QWORD [rsp + 40] ; fd
;; the buffer is our current dictionnary addr + 4
    mov rsi, QWORD [rsp + 56]
    add rsi, 4
;; the len is our current dictionnary + 2, dereferenced
    mov r10, QWORD [rsp + 56] ; take current dictionnary
    add r10, 2 ; go to offset 2
    xor rdx, rdx
    mov dx, WORD [r10] ; take the value
    syscall

;; decremente the multiplicator
    dec QWORD [rsp + 48]
    jmp _write_word_on_file.loop

_increment_compressed_datas_offset:
    add QWORD [rsp + 32], 4 ; go to next compressed datas struct
    jmp _find_and_write.loop

_close_new_file:
    mov rax, 3
    mov rdi, QWORD [rsp + 40]
    syscall

_decompress_end:
    leave
    ret
