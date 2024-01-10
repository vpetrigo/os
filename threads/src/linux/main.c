#include <unistd.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>

// MACROS

#define ARRAY_SIZE(arr) sizeof(arr) / sizeof(arr[0])
#define UNUSED_PARAM __attribute__((unused))

// %rsp	stack pointer	Yes
// %rsi used to pass 2nd argument to functions No
// %rdi used to pass 1st argument to functions No
// %rbp	callee-saved; base pointer	Yes
// %rbx callee-saved Yes
// %r12-r15 callee-saved registers Yes

// TYPES

struct stack_frame
{
    uint64_t rflags;
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t rbp;
    uint64_t rbx;
    uint64_t rip;
};

struct thread
{
    void *context;
};

typedef void (*thread_entry_f)(void);

// PRIVATE VARIABLES

struct thread *threads[2];

// FUNCTION DEFINITIONS

struct thread *thread_create(thread_entry_f handler)
{
    const size_t default_stack_size = 1024;
    const size_t to_allocate = default_stack_size + sizeof(struct thread);
    uint8_t *memory = calloc(1, to_allocate);

    if (memory == NULL) {
        return NULL;
    }

    const size_t stack_frame_offset = to_allocate - sizeof(struct stack_frame);
    struct thread thread = {0};
    struct stack_frame frame = {0};
    // |------------------| <--- struct thread
    // |------------------| <--- stack (start)
    // |                  |
    // |    thread mem    |
    // |                  |
    // |------------------| <--- stack (end) (+ default_stack_size)
    // |------------------| <--- + struct stack_frame
    frame.rip = (uint64_t)handler;
    thread.context = memory + stack_frame_offset;
    memcpy(memory, &thread, sizeof(thread));
    memset(memory + sizeof(thread), 0xAB, default_stack_size - sizeof(struct stack_frame));
    memcpy(memory + stack_frame_offset, &frame, sizeof(frame));

    return (struct thread *)memory;
}

void thread_destroy(struct thread *thread) {
    if (thread == NULL) {
        return;
    }

    free(thread);
}

void thread_switch(struct thread *from, struct thread *to) {
    extern void switch_threads_arch(void **prev_ctx, void *next_ctx);

    switch_threads_arch(&from->context, to->context);
}

noreturn void handler1(void) {
    uint8_t counter = 1;

    for (;;) {
        // push rip = handler1
        // push rbp, rdx, r12-15
        printf("Hello from thread 1\n");
        // pop rbp, rdx, r12-15
        // pop rip = handler1
        sleep(2);

        if (counter == 3) {
            counter = 0;
            thread_switch(threads[0], threads[1]);
        }

        ++counter;
    }
}

noreturn void handler2(void) {
    uint8_t counter = 1;

    for (;;) {
        printf("Hello from thread 2\n");
        sleep(2);

        if (counter == 3) {
            counter = 0;
            thread_switch(threads[1], threads[0]);
        }

        ++counter;
    }
}

int main() {
    threads[0] = thread_create(handler1);
    threads[1] = thread_create(handler2);

    struct stack_frame main_frame = {0};
    struct thread main_thread = {
        .context = &main_frame,
    };

    printf("Allocated thread 1 address: %p\n", threads[0]);
    printf("Allocated thread 2 address: %p\n", threads[1]);
    thread_switch(&main_thread, threads[0]);
    thread_destroy(threads[0]);
    thread_destroy(threads[1]);

    return 0;
}
