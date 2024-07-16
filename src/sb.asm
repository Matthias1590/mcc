%include "std.inc"
%include "sys.inc"
%include "io.inc"

%define SB_NO_FUNCS
%include "sb.inc"

section .text
    global sb_new
    global sb_free
    global sb_add

; <-
; todo
; ->
; rax - ptr_sb
sb_new:
    mov rdi, struct_sb_size
    call alloc

    cmp rax, 0
    je .return

    push rax

    mov qword [rax + struct_sb.capacity], 1
    mov qword [rax + struct_sb.length], 0

    mov rdi, 1
    call alloc

    pop rdx
    
    cmp rax, 0
    je .return

    mov qword [rdx + struct_sb.data], rax
    mov byte [rax], 0
    mov rax, rdx

.return:
    ret

; <-
; rdi - ptr_sb
; ->
; todo
sb_free:
    push rdi

    mov rdi, qword [rdi + struct_sb.data]
    call free

    pop rdi
    call free

    ret

; <-
; rdi - ptr_sb
; rsi - charPtr_string
; ->
; todo
sb_add:
    push rsi
    ; r15 - ptr_sb
    mov r15, rdi
    ; r9 - u64_capacity
    mov r9, qword [r15 + struct_sb.capacity]

    ; r10 - strlen(charPtr_string)
    mov rdi, rsi
    call strlen
    mov r10, rax

    ; r11 = length + new_length
    mov r11, qword [r15 + struct_sb.length]
    add r11, r10

    ; length = length + new_length
    mov qword [r15 + struct_sb.length], r11

    ; length + new_length + 1 >= capacity
    inc r11
    cmp r11, r9
    jb .copy

    ; realloc
    mov rdi, qword [r15 + struct_sb.data]  ; rdi = sb data ptr
    mov rsi, r9   ; rsi = sb capacity * 2
    shl rsi, 1
    call realloc
    mov qword [r15 + struct_sb.data], rax  ; sb data ptr = rax

.copy:
    pop rsi

    ; go to the end of the string in rax
    ; add rax, r11

    ; copy bytes from [rsi + offset] to [rax + offset]
.copy_loop:
    cmp byte [rsi], 0
    je .copy_end
    mov bl, byte [rsi]
    mov byte [rax], bl
    inc rsi
    inc rax
    jmp .copy_loop

.copy_end:
    mov byte [rax], 0

    ret
