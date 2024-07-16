section .text
    global write
    global mmap
    global munmap
    global exit

; <-
; rdi - u32_fd
; rsi - charPtr_buf
; rdx - u64_count
; ->
; ??? - ?????
write:
    mov rax, 1
    syscall
    ret

; <-
; todo
; ->
; todo
mmap:
    mov rax, 9
    syscall
    ret

; <-
; todo
; ->
; todo
munmap:
    mov rax, 11
    syscall
    ret

; <-
; rdi - u8_exitCode
; ->
; ??? - ?????
exit:
    mov rax, 60
    syscall
    ret
