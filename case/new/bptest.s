main:
    irmovq $32, %rax
    irmovq $0, %rbx
    irmovq $1, %rcx
    irmovq $1, %rdx
    jmp .L2
.L1:
    addq %rcx, %rbx
    addq %rdx, %rcx
.L2:
    rrmovq %rcx, %rdx
    subq %rax, %rdx
    jle .L1

    halt