#include "thread.h"

#include <signal.h>
#include <unistd.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>

// MACROS

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))
#define UNUSED_PARAM __attribute__((unused))

// TYPES

typedef void (*thread_entry_f)(void);

// PRIVATE VARIABLES

// struct stack_frame main_frame = {0};
struct thread main_thread = {
    .context = NULL,
};
static bool should_stop = false;

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

void *get_pc(void) {
    return __builtin_return_address(0);
}

void assert(bool expression) {
    if (!expression) {
        __asm__("int3");
    }
}

int main(void) {
    thread_scheduler_init(&main_thread);
    struct thread *threads[10];

    for (size_t i = 0; i < ARRAY_SIZE(threads); ++i) {
        threads[i] = thread_create(handler1);
    }

    for (size_t i = 0; i < ARRAY_SIZE(threads); ++i) {
        thread_start(threads[i]);
    }

    alarm(1);
    printf("Thread main address: %p\n", &main_thread);
    // // TODO: check how to setup proper offset to jump over the `thread_switch()` call
    // main_frame.rip = (uint64_t)(get_pc()) + 1;

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
