%include "sys.inc"

section .text
    global strlen
    global alloc
    global free
    global realloc

; <-
; rdi - charPtr_string
; ->
; rax - u64_length
strlen:
    mov rax, 0

.loop:
    cmp byte [rdi], 0
    je .end
    inc rax
    inc rdi
    jmp .loop

.end:
    ret

; <-
; rdi - u64_size
; ->
; rax - ptr_data
alloc:
    add rdi, 8  ; add 8 bytes to the size, because we store the size infront of the pointer
    push rdi

    mov rsi, rdi
    mov rdi, 0
    mov rdx, 3  ; PROT_READ | PROT_WRITE
    mov r10, 34  ; MAP_PRIVATE | MAP_ANONYMOUS
    mov r8, -1
    mov r9, 0
    call mmap

    pop rdi
    
    cmp rax, -1
    je .failed

    mov qword [rax], rdi  ; store the size
    add rax, 8  ; shift away the 8 bytes containing the size
    ret

.failed:
    mov rax, 0
    ret

; <-
; rdi - ptr_data
; ->
; ??? - ?????
free:
    mov rsi, qword [rdi - 8]  ; read the size which is stored 8 bytes infront of the pointer
    sub rdi, 8
    call munmap

    ret

; <-
; rdi - ptr_data
; rsi - u64_newSize
; ->
; rax - ptr_newData
realloc:
    push rdi

    ; rax = rcx = alloc(u64_newSize)
    mov rdi, rsi
    call alloc

    ; rdi = ptr_data
    pop rdi

    push rax

    ; rbx = ptr_data
    mov rbx, rdi

    ; rdx = u64_oldSize
    mov rdx, qword [rdi - 8]

    ; copy rdx bytes from [rdi + offset] to [rax + offset]
.copy_loop:
    cmp rdx, 0
    je .copy_end
    mov r8b, byte [rdi]
    mov byte [rax], r8b
    inc rdi
    inc rax
    dec rdx

.copy_end:
    mov rdi, rbx
    call free

    pop rax
    ret
