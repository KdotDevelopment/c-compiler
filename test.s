global main
section .text
  main:
    call _main
    mov rdi, rax
    mov rax, 60
    syscall
  _print:
    push rbp
    mov rbp, rsp
    sub rsp, 64
    mov rax, 1
    mov r14, 65
    add r14, r15
    mov -8[rbp], r14
    mov edi, 1
    mov rdx, 1
    lea rsi, -8[rbp]
    syscall
    mov -8[rbp], 10
    mov edi, 1
    mov rdx, 1
    lea rsi, -8[rbp]
    syscall
    mov r8, 4
    mov rax, r8
    mov rsp, rbp
    pop rbp
    ret
  _main:
    push rbp
    mov rbp, rsp
    sub rsp, 64
    mov r8, 0
    mov DWORD -4[rbp], r8d
    mov r8, 0
    mov DWORD -8[rbp], r8d
    jmp L0
  L0:
    mov r8d, DWORD -4[rbp]
    mov r9, 1
    add r8, r9
    mov DWORD -4[rbp], r8d
    mov r15, DWORD -4[rbp]
    call _print
    mov r8, rax
    mov r8d, DWORD -8[rbp]
    mov r9, 1
    add r8, r9
    mov DWORD -8[rbp], r8d
    mov r8d, DWORD -8[rbp]
    mov r9, 10
    cmp r8, r9
    setl al
    movzx r9, al
    cmp r9, 0
    jne L0
    je L1
  L1:
    mov r8d, DWORD -4[rbp]
    mov rax, r8
    mov rsp, rbp
    pop rbp
    ret
