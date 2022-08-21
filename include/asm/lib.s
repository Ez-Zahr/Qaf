read_line:
    pushq %rbp
    movq %rsp, %rbp
_read_line_loop:
    movq %r8, %rsi
    movq $1, %rdx
    movq $0, %rax
    movq $0, %rdi
    syscall
    movzbq (%r8), %rbx
    cmpq $10, %rbx
    je _read_line_exit
    incq %r8
    jmp _read_line_loop
_read_line_exit:
    movb $0, (%r8)
    leaveq
    retq

print_str:
    pushq %rbp
    movq %rsp, %rbp
_print_str_loop:
    movzbq (%r8), %rbx
    cmpq $0, %rbx
    je _print_str_exit
    movq %r8, %rsi
    movq $1, %rdx
    movq $1, %rax
    movq $1, %rdi
    syscall
    incq %r8
    jmp _print_str_loop
_print_str_exit:
    pushq $10
    movq %rsp, %rsi
    movq $1, %rdx
    movq $1, %rax
    movq $1, %rdi
    syscall
    leaveq
    retq

print_int:
    pushq %rbp
    movq %rsp, %rbp
    subq $16, %rsp
    movq $10, -8(%rbp)
    movq $0, -16(%rbp)
    movq %r8, %rax
    cmpq $0, %rax
    jge _print_int_convert
    negq %rax
    pushq %rax
    pushq $45
    movq %rsp, %rsi
    movq $1, %rdx
    movq $1, %rax
    movq $1, %rdi
    syscall
    addq $8, %rsp
    popq %rax
_print_int_convert:
    movq $0, %rdx
    idivq -8(%rbp)
    addq $48, %rdx
    pushq %rdx
    incq -16(%rbp)
    cmpq $0, %rax
    jne _print_int_convert
_print_int_write:
    movq %rsp, %rsi
    movq $1, %rdx
    movq $1, %rax
    movq $1, %rdi
    syscall
    addq $8, %rsp
    decq -16(%rbp)
    cmpq $0, -16(%rbp)
    jg _print_int_write
    pushq $10
    movq %rsp, %rsi
    movq $1, %rdx
    movq $1, %rax
    movq $1, %rdi
    syscall
    leaveq
    retq

copy_str:
    pushq %rbp
    movq %rsp, %rbp
_copy_str_loop:
    movzbq (%r8), %rbx
    movb %bl, (%r9)
    movzbq (%r8), %rbx
    cmpq $0, %rbx
    je _copy_str_exit
    incq %r8
    incq %r9
    jmp _copy_str_loop
_copy_str_exit:
    leaveq
    retq
