/* 
Ivan Chowdhury
ECE 357: Computer Operating Systems
Fall 2018
Semaphore Operation Implementation [Header]
*/

#include <unistd.h>
#include <sys/types.h>

#define N_PROC 64   // Max number of virtual processors

struct sem
{
    char lock;  // Mutex locking
    int count;  // Semaphore counter  
    pid_t wpid[N_PROC];  // Array of waiting processes for the semaphor
    int wp;  // Index of wpid
};


void sem_init(struct sem *s, int count);
//   Initialize the semaphore *s with the initial count. Initialize
//   any underlying data structures.  sem_init should only be called
//   once in the program (per semaphore).  If called after the
//   semaphore has been used, results are unpredictable.

int sem_try(struct sem *s);
//   Attempt to perform the "P" operation (atomically decrement
//   the semaphore).  If this operation would block, return 0,
//   otherwise return 1.

void sem_wait(struct sem *s);
//   Perform the P operation, blocking until successful.

void sem_inc(struct sem *s);
//   Perform the V operation.  If any other tasks were sleeping
//   on this semaphore, wake them by sending a SIGUSR1 to their
//   process id (which is not the same as the virtual processor number).
//   If there are multiple sleepers (this would happen if multiple
//   virtual processors attempt the P operation while the count is <1)
//   then \fBall\fP must be sent the wakeup signal.