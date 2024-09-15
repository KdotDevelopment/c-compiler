global main
section .text
  main:
    mov r8, 9
    mov r9, 5
    mov rax, r8
    mov rbx, r9
    div rbx
    mov r9, rdx
    mov r8, 4
    mov rax, r8
    mul r9
    mov r9, rax
    mov rdi, r9
    mov rax, 60
    syscall
