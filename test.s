global main
section .text
  main:
    call _main
    mov rdi, rax
    mov rax, 60
    syscall
  _test:
    push rbp
    mov rbp, rsp
    sub rsp, 64
    mov r8, 4
    mov rax, r8
    mov rsp, rbp
    pop rbp
    ret
  _main:
    push rbp
    mov rbp, rsp
    sub rsp, 64
    mov r8, 12
    mov BYTE -1[rbp], r8b
    mov r8, 10
    mov DWORD -8[rbp], r8d
    mov r8, 3
    mov DWORD -12[rbp], r8d
    mov r8b, BYTE -1[rbp]
    mov r9d, DWORD -8[rbp]
    add r8, r9
    mov r9d, DWORD -12[rbp]
    mov rax, r9
    mul r8
    mov r8, rax
    push r8
    call _test
    mov r9, rax
    pop r8
    add r8, r9
    mov DWORD -16[rbp], r8d
    mov r8d, DWORD -16[rbp]
    mov rax, r8
    mov rsp, rbp
    pop rbp
    ret
