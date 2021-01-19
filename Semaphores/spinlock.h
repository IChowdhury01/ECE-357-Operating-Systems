/* 
Ivan Chowdhury
ECE 357: Computer Operating Systems
Fall 2018
Implementation of Spin Lock [Header]
*/

#include <sched.h>

#include "tas.h"

void spin_lock(volatile char *lock);
void spin_unlock(volatile char *lock);