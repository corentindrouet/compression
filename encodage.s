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

_start:
    enter 32, 0

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

;; Open our new file
    mov rax, 1
    lea rdi, [rel _new_file.string]
    mov rsi, 578
    mov rdx, 493
    syscall
    cmp rax, 0
    jle _end
    mov QWORD [rsp + 8], rax

;; Just mmap the file to compress, travel it a first time,
;; to create the dictionnary in a new file, then
;; travel it again, and replace word according to the dictionnary.

_end:
    leave
    mov rax, 60
    mov rdi, 0
    syscall
