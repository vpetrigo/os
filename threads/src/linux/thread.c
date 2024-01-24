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

static void scheduler_tick(int sig_num) {
    struct thread *to_execute = NULL;

    if (!list_empty(&ready)) {
        // execute something
        to_execute = (struct thread *)ready.next;
        list_del(&to_execute->node);
    }
    else {
        printf("Nothing to execute\n");
    }

    if (to_execute != NULL) {
        // printf("Is thread 1 %d\n", thread1 == to_execute);
        // printf("Is thread 2 %d\n", thread2 == to_execute);
        list_add_tail(&to_execute->node, &ready);
    }

    alarm(1);
    thread_switch(current_thread, to_execute);
}

static void thread_enter(struct thread *this, thread_entry_f this_handler) {
    printf("Thread enter: %p\n", this);
    exit(1);
    current_thread = this;

    this_handler();
}

// FUNCTION DEFINITIONS

void thread_temp_set_current_thread(struct thread *thread) {
    current_thread = thread;
}

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

void thread_scheduler_init(struct thread *main_thread) {
    struct sigaction sa = {0};

    sa.sa_handler = scheduler_tick;
    sa.sa_flags = SA_NODEFER;

    sigaction(SIGALRM, &sa, NULL);
    list_init(&ready);
    current_thread = main_thread;
}
