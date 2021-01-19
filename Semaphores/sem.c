/* 
Ivan Chowdhury
ECE 357: Computer Operating Systems
Fall 2018
Semaphore Operation Implementation
*/

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>

#include "spinlock.h"
#include "sem.h"

void sem_init(struct sem *s, int count) {

    s->lock = 0;    // Initialize mutex locking
    
    spin_lock(&s->lock);    
    // START CRITICAL REGION

    s->count = count;
    s->wp = 0;

    // END CRITICAL REGION
    spin_unlock(&s->lock);  
}

int sem_try(struct sem *s) {
    
    spin_lock(&(s->lock));  
    // START CRITICAL REGION

    if (s->count > 0) {
        s->count--;
        
        // END CRITICAL REGION  
        spin_unlock(&(s->lock));    
        return 1;
    }
    else {
        spin_unlock(&(s->lock));
        return 0;
    }
}

void sem_wait(struct sem *s) {
    // Signal masks
    sigset_t mask, new_mask;    
    sigemptyset(&new_mask); // Initialize empty signal mask
    sigaddset(&new_mask, SIGUSR1); // Add SIGUSR1 to blocked signals */
    sigprocmask(SIG_BLOCK, &new_mask, &mask);   // Modify signal mask


    spin_lock(&s->lock);
    // START CRITICAL REGION

    while (s->count == 0) { // Loop while process is blocked
    
		(s->wpid)[(s->wp)++] = getpid(); // Put process on waitlist
        s->wp++;

        // END CRITICAL REGION
        spin_unlock(&s->lock);

        sigfillset(&new_mask);
        sigdelset(&new_mask, SIGUSR1); 
        sigsuspend(&new_mask); // Block signal
        
        spin_lock(&s->lock);
        // START CRITICAL REGION
    }

    sigprocmask(SIG_SETMASK, &mask, NULL);  // Return original signal mask
    s->count--;    // Decrement semaphore counter

    // END CRITICAL REGION
    spin_unlock(&s->lock);
}

void sem_inc(struct sem *s) {
    spin_lock(&(s->lock));
    // START CRITICAL REGION

    s-> count += 1;

    if (s->count == 1) {    // When semaphore count becomes 1, send SIGUSR1 to wake all processes
        for (int i = 0 ; i < s->wp; ++i)
            kill(s->wpid[i], SIGUSR1); 
        s->wp = 0;
    }

    // END CRITICAL REGION
    spin_unlock(&(s->lock));
}