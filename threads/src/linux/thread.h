/**
 * \file
 * \brief
 * \author
 */

#ifndef THREAD_H
#define THREAD_H

#include "list.h"

#include <stdint.h>

// PUBLIC TYPE DECLARATIONS

typedef void (*thread_entry_f)(void *);

enum thread_state
{
    THREAD_ERROR = 0,
    THREAD_ACTIVE,
    THREAD_FINISHED,
    THREAD_BLOCKED,
    THREAD_DONE,
};

struct thread
{
    struct list_head node;
    enum thread_state state;
    void *context;
};

// for x64 platform
// %rsp	stack pointer	Yes
// %rsi used to pass 2nd argument to functions No
// %rdi used to pass 1st argument to functions No
// %rbp	callee-saved; base pointer	Yes
// %rbx callee-saved Yes
// %r12-r15 callee-saved registers Yes

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

// FUNCTION DECLARATIONS

struct thread *thread_create(thread_entry_f handler, void *arg);
void thread_start(struct thread *thread);
void thread_destroy(struct thread *thread);
void thread_switch(struct thread *from, struct thread *to);
void thread_scheduler_init(struct thread *main_thread);

void thread_scheduler_call(void);

void thread_scheduler_preemtpion_enable(void);
void thread_scheduler_preemtpion_disable(void);

#endif /* THREAD_H */
