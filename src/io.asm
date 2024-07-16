%include "std.inc"
%include "sys.inc"

section .text
    global puts
    global putn

; <-
; rdi - charPtr_string
; ->
; ??? - ?????
puts:
    mov rsi, rdi

    ; u64_length = strlen(charPtr_string)
    call strlen
    mov rdx, rax

    mov rdi, 1
    call write
    ret

; <-
; rdi - u64_num
; ->
; ??? - ?????
putn:
    push rdi
    push rdx
    push rax

    cmp rdi, 10
    jb .below_10

    ; calculate u64_num / 10 and u64_num % 10
    mov rax, rdi
    xor rdx, rdx
    mov rcx, 10
    div rcx

    mov rdi, rax
    call putn

    mov rdi, rdx
.below_10:
    ; char_putnChar = u64_num + '0'
    add rdi, '0'
    mov byte [char_putnChar], dil

    mov rdi, 1
    mov rsi, char_putnChar
    mov rdx, 1
    call write

    pop rax
    pop rdx
    pop rdi
    ret

section .bss
    char_putnChar: resb 1
