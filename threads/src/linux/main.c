#include "spin_lock.h"
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

// PRIVATE VARIABLES

// struct stack_frame main_frame = {0};
struct thread main_thread = {
    .context = NULL,
};
static bool should_stop = false;

static struct thread *idle_thread;

static struct thread *thread1;
static struct thread *thread2;
static struct spin_lock *thread_lock;

// TODO: add generic thread_entry function
// void thread_entry(struct thread *thread) {
//     ...
//     current_thread = thread;
//     handler = // get current_thread handler
//     handler();
// }

// FUNCTION DEFINITIONS

void handler(void *arg) {
    const char *str = arg;
    size_t counter = 0;

    for (;;) {
        if (strcmp("a", str) == 0) {
            spin_lock_lock(thread_lock);
        }

        printf("Hello from thread '%s'\n", str);
        sleep(2);

        if (strcmp("a", str) == 0 && counter == 10) {
            counter = 0;
            spin_lock_unlock(thread_lock);
            sleep(2);
        }

        ++counter;
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
    const char *ids[] = {
        "a", "b", "c", "d", "e", "f", "g", "h", "i", "j",
    };

    struct thread *threads[2];

    for (size_t i = 0; i < ARRAY_SIZE(threads); ++i) {
        threads[i] = thread_create(handler, (void *)ids[i]);
    }

    for (size_t i = 0; i < ARRAY_SIZE(threads); ++i) {
        thread_start(threads[i]);
    }

    printf("Thread main address: %p\n", &main_thread);
    // TODO: check how to setup proper offset to jump over the `thread_switch()` call
    // main_frame.rip = (uint64_t)(get_pc()) + 1;

    for (;;) {
        thread_scheduler_call();

        printf("In the idle thread!\n");
        sleep(2);
        if (should_stop) {
            break;
        }
    }

    printf("Goodbye!");
    // thread_destroy(threads[0]);
    // thread_destroy(threads[1]);

    return 0;
}
