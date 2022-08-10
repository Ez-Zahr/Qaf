print:
    pushq %rbp
    movq %rsp, %rbp
    movq %r8, %rsi
    movq %r9, %rdx
    movq $1, %rax
    movq $1, %rdi
    syscall
    leaveq
    retq

print_newline:
    pushq %rbp
    movq %rsp, %rbp
    pushq $10
    movq %rsp, %r8
    movq $1, %r9
    callq print
    leaveq
    retq

print_int:
    pushq %rbp
    movq %rsp, %rbp
    subq $16, %rsp
    movq $10, -8(%rbp)
    movq $0, -16(%rbp)
    movq %r8, %rax
_loop:
    movq $0, %rdx
    idivq -8(%rbp)
    addq $48, %rdx
    pushq %rdx
    incq -16(%rbp)
    cmpq $0, %rax
    jne _loop
_print:
    movq %rsp, %r8
    movq $1, %r9
    callq print
    addq $8, %rsp
    decq -16(%rbp)
    cmpq $0, -16(%rbp)
    jg _print
    callq print_newline
_return:
    leaveq
    retq
