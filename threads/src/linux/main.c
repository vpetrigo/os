#include "list.h"

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
    struct list_head node;
    void *context;
};

typedef void (*thread_entry_f)(void);

// PRIVATE VARIABLES

struct stack_frame main_frame = {0};
struct thread main_thread = {
    .context = &main_frame,
};
static bool should_stop = false;


static struct list_head ready;
static struct thread *current_thread;
static struct thread *idle_thread;

static struct thread *thread1;
static struct thread *thread2;

// TODO: add generic thread_entry function
// void thread_entry(struct thread *thread) {
//     ...
//     current_thread = thread;
//     handler = // get current_thread handler
//     handler();
// }

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

void thread_start(struct thread *thread) {
    list_add_tail(&thread->node, &ready);
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
    current_thread = thread1;

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
    current_thread = thread2;

    for (;;) {
        printf("Hello from thread 2\n");
        sleep(2);
    }
}

void handler(int sig_num) {
    struct thread *to_execute = NULL;

    if (!list_empty(&ready)) {
        // execute something
        to_execute = (struct thread *)ready.next;
        list_del(&to_execute->node);
    } else {
        printf("Nothing to execute\n");
    }

    if (to_execute != NULL) {
        printf("Is thread 1 %d\n", thread1 == to_execute);
        printf("Is thread 2 %d\n", thread2 == to_execute);
        list_add_tail(&to_execute->node, &ready);
    }

    alarm(1);
    thread_switch(current_thread, to_execute);
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
    list_init(&ready);
    thread1 = thread_create(handler1);
    thread2 = thread_create(handler2);
    thread_start(thread1);
    thread_start(thread2);
    alarm(1);
    // // TODO: check how to setup proper offset to jump over the `thread_switch()` call
    // main_frame.rip = (uint64_t)(get_pc()) + 1;
    current_thread = &main_thread;

    for (;;) {
        if (should_stop) {
            break;
        }

        sleep(2);
    }

    printf("Goodbye!");
    // thread_destroy(threads[0]);
    // thread_destroy(threads[1]);

    return 0;
}
