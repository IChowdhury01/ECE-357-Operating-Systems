/* 
Ivan Chowdhury
ECE 357: Computer Operating Systems
Fall 2018
Implementation of Spin Lock
*/

#include <sched.h>

#include "tas.h"
#include "spinlock.h"

void spin_lock(volatile char *lock) {
	while (tas(lock))
		sched_yield();  // Yields the processor. Always succeeds
}

void spin_unlock(volatile char *lock) {
	*lock = 0;
}