global main
section .text
  main:
    push rbp
    mov rbp, rsp
    mov r8, 10
    mov QWORD -8[rbp], r8
    mov QWORD r9, -8[rbp]
    mov r10, 2
    mov r11, 7
    mov rax, r11
    mul r10
    mov r10, rax
    add r9, r10
    mov QWORD -8[rbp], r9
    mov QWORD r10, -8[rbp]
    mov rdi, r10
    mov rax, 60
    syscall
