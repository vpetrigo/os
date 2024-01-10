#include <signal.h>
#include <unistd.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>

// MACROS

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))
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

struct stack_frame main_frame = {0};
struct thread main_thread = {
    .context = &main_frame,
};
static struct thread *threads[2];
static bool should_stop = false;

// FUNCTION DEFINITIONS

struct thread *thread_create(thread_entry_f handler)
{
    const size_t default_stack_size = 4096;
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
    for (;;) {
        // push rip = handler1
        // push rbp, rdx, r12-15
        printf("Hello from thread 1\n");
        // pop rbp, rdx, r12-15
        // pop rip = handler1
        sleep(2);
    }
}

noreturn void handler2(void) {
    for (;;) {
        printf("Hello from thread 2\n");
        sleep(2);
    }
}

void handler(int sig_num) {
    static size_t counter = 0;
    static size_t iteration = 0;

    if (sig_num != SIGALRM) {
        return;
    }

    const size_t current_thread_id = counter % ARRAY_SIZE(threads);
    const size_t next_thread_id = (counter + 1) % ARRAY_SIZE(threads);

    ++iteration;
    ++counter;
    printf("Schedule switching from %zu to %zu\n", current_thread_id, next_thread_id);
    //    thread_switch(threads[current_thread_id], threads[next_thread_id]);

    if (iteration < 2) {
        alarm(2);
        thread_switch(threads[current_thread_id], threads[next_thread_id]);
    }
    else {
        should_stop = true;
        thread_switch(threads[current_thread_id], &main_thread);
    }
}

void *get_pc(void) {
    return __builtin_return_address(0);
}

void assert(bool expression) {
    if (!expression) {
        __asm__("int3");
    }
}

int main() {
    struct sigaction sa = {0};

    sa.sa_handler = handler;
    sa.sa_flags = SA_NODEFER;

    sigaction(SIGALRM, &sa, NULL);
    threads[0] = thread_create(handler1);
    threads[1] = thread_create(handler2);

    printf("Allocated thread 1 address: %p\n", threads[0]);
    printf("Allocated thread 2 address: %p\n", threads[1]);
    alarm(1);
    // TODO: check how to setup proper offset to jump over the `thread_switch()` call
    main_frame.rip = (uint64_t)(get_pc()) + 1;
    thread_switch(&main_thread, threads[0]);

    for (;;) {
        if (should_stop) {
            break;
        }

        sleep(2);
    }

    printf("Goodbye!");
    thread_destroy(threads[0]);
    thread_destroy(threads[1]);

    return 0;
}
