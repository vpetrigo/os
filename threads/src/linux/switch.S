    .text
    .code64

    .extern thread_enter

    .type thread_prelude, @function
    .global thread_prelude
thread_prelude:
    movq %r15, %rdi
    movq %r14, %rsi
    movq %r13, %rdx
    call thread_enter

    .type switch_threads_arch, @function
    .global switch_threads_arch

switch_threads_arch:
    pushq %rbx
    pushq %rbp
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
    pushfq

    movq %rsp, (%rdi)
    movq %rsi, %rsp

    popfq
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %rbp
    popq %rbx

    retq
