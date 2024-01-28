/**
 * \file
 * \brief
 * \author
 */

#include "spin_lock.h"

#include "thread.h"

struct spin_lock
{
    // core1 lock
    // core2 lock
};

// PUBLIC FUNCTION DEFINITIONS

void spin_lock_init(struct spin_lock *lock) {
    (void)lock;
}

void spin_lock_lock(struct spin_lock *lock) {
    (void)lock;
    thread_scheduler_preemtpion_disable();
}

void spin_lock_unlock(struct spin_lock *lock) {
    (void)lock;
    thread_scheduler_preemtpion_enable();
}
