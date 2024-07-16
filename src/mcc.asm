%include "sys.inc"
%include "io.inc"
%include "std.inc"
%include "sb.inc"

section .text
    global _start

_start:
    call sb_new
    mov r14, rax

    push r14
    mov rdi, r14
    mov rsi, msg
    call sb_add

    ; pop r14
    ; push r14
    ; mov rdi, qword [r14 + struct_sb.data]
    ; call puts

    pop r14
    push r14
    mov rdi, r14
    mov rsi, msg2
    call sb_add

    pop r14
    push r14
    mov rdi, qword [r14 + struct_sb.data]
    call puts

    pop r14
    push r14
    mov rdi, r14
    call sb_free

    mov rdi, 0
    call exit

section .data
    msg: db "hey", 10, 0
    msg2: db "aa", 10, 0

section .bss
    my_sb: resq 1
