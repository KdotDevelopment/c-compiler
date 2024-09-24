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
    sub rsp, 64
    mov r8, 12
    mov BYTE -1[rbp], r8b
    mov r8, 10
    mov WORD -4[rbp], r8w
    mov r8b, BYTE -1[rbp]
    mov r9w, WORD -4[rbp]
    add r8, r9
    mov DWORD -8[rbp], r8d
    mov r8d, DWORD -8[rbp]
    mov rax, r8
    mov rsp, rbp
    pop rbp
    ret
