/**
 * \file
 * \brief
 * \author
 */
#include "thread.h"

#include <signal.h>
#include <unistd.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct list_head ready;
static struct thread *current_thread;

// PRIVATE FUNCTION DEFINITIONS

static void thread_set_current(struct thread *this) {
    current_thread = this;
}

static void scheduler_tick(int sig_num) {
    struct thread *to_execute = NULL;
    struct thread *me = current_thread;
    printf(" > scheduler in %p thread\n", current_thread);

    if (!list_empty(&ready)) {
        // execute something
        to_execute = (struct thread *)ready.next;
        list_del(&to_execute->node);
    }
    else {
        printf("Nothing to execute\n");
    }

    if (to_execute != NULL) {
        list_add_tail(&to_execute->node, &ready);
    }

    alarm(1);
    printf(" - switch from %p to %p\n", current_thread, to_execute);
    thread_switch(current_thread, to_execute);
    printf(" + return to context %p\n", me);
    thread_set_current(me);
}

void thread_enter(struct thread *this, thread_entry_f this_handler, void *arg) {
    thread_set_current(this);
    this_handler(arg);
}

// FUNCTION DEFINITIONS

struct thread *thread_create(thread_entry_f handler, void *arg)
{
    extern void thread_prelude(void);
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
    frame.rip = (uint64_t)thread_prelude;
    frame.r15 = (uint64_t)memory;
    frame.r14 = (uint64_t)handler;
    frame.r13 = (uint64_t)arg;
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

void thread_scheduler_init(struct thread *main_thread) {
    struct sigaction sa = {0};

    sa.sa_handler = scheduler_tick;
    sa.sa_flags = SA_NODEFER;

    sigaction(SIGALRM, &sa, NULL);
    list_init(&ready);
    current_thread = main_thread;
}
