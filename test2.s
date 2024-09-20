global main
section .text
  main:
    push rbp
    mov rbp, rsp
    mov r8, 2
    mov r9, 2
    cmp r8, r9
    setz al
    mov QWORD -8[rbp], r9
    mov QWORD r8, -8[rbp]
    movzx rdi, al
    mov rax, 60
    syscall
