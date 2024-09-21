global main
section .text
  main:
    call _main
    mov rdi, rax
    mov rax, 60
    syscall
  _main:
    push rbp
    mov rbp, rsp
    mov r8, 20
    mov QWORD -8[rbp], r8
    mov QWORD r8, -8[rbp]
    mov r9, 20
    cmp r8, r9
    setg al
    movzx r9, al
    cmp r9, 0
    je L0
    mov r8, 200
    mov QWORD -8[rbp], r8
    jmp L0
  L0:
    mov QWORD r8, -8[rbp]
    mov rax, r8
    pop rbp
    ret
