/**
 * \file
 * \brief
 * \author
 */

#ifndef THREADS_SPIN_LOCK_H
#define THREADS_SPIN_LOCK_H

// PUBLIC TYPES

struct spin_lock {
};

// PUBLIC FUNCTION DECLARATIONS

void spin_lock_init(struct spin_lock *lock);
void spin_lock_lock(struct spin_lock *lock);
void spin_lock_unlock(struct spin_lock *lock);

#endif /* THREADS_SPIN_LOCK_H */
