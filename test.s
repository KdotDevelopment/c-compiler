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
    mov -8[rbp], rdi
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
    mov rdi, DWORD -4[rbp]
    add rdi, 'A'
    mov r8d, DWORD -4[rbp]
    mov r9, 1
    add r8, r9
    mov DWORD -4[rbp], r8d
    call _print
    mov r8, rax
    mov r8d, DWORD -8[rbp]
    mov r9, 5
    mov rax, r8
    mov rbx, r9
    div rbx
    mov r9, rdx
    mov r8, 0
    cmp r9, r8
    setz al
    movzx r8, al
    cmp r8, 0
    je L2
    mov rdi, 10
    call _print
    mov r9, rax
    jmp L2
  L2:
    mov r8d, DWORD -8[rbp]
    mov r9, 1
    add r8, r9
    mov DWORD -8[rbp], r8d
    mov r8d, DWORD -8[rbp]
    mov r9, 30
    cmp r8, r9
    setl al
    movzx r9, al
    cmp r9, 0
    jne L0
    je L1
  L1:
    mov rdi, 'H'
    call _print
    mov r8, rax
    mov rdi, 'e'
    call _print
    mov r8, rax
    mov rdi, 'l'
    call _print
    mov r8, rax
    mov rdi, 'l'
    call _print
    mov r8, rax
    mov rdi, 'o'
    call _print
    mov r8, rax
    mov rdi, '\n'
    call _print
    mov r8, rax
    mov rax, 1
    mov -8[rbp], 65
    mov -7[rbp], 66
    mov -6[rbp], 67
    mov edi, 1
    mov rdx, 3
    lea rsi, -8[rbp]
    syscall
    mov rdi, 10
    call _print
    mov r8, rax
    mov r8d, DWORD -4[rbp]
    mov rax, r8
    mov rsp, rbp
    pop rbp
    ret
