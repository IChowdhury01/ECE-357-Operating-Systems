/* 
Ivan Chowdhury
ECE 357: Computer Operating Systems
Fall 2018
FIFO Implementation [Header]
*/

#include "sem.h"

#define MYFIFO_BUFSIZ 4096  // Length of FIFO

struct fifo {
    // Semaphores
    struct sem s1;  // Filled/Empty 
    struct sem s2; 
    struct sem srw; // Read/Write
    
    int rd, wr; // Index for next read/write
    unsigned long FIFObuf[MYFIFO_BUFSIZ];

};

void fifo_init(struct fifo *f);
//    Initialize the shared memory FIFO *f including any
//    required underlying initializations (such as calling sem_init)
//    The FIFO will have a fifo length of MYFIFO_BUFSIZ elements,
//    which should be a static #define in fifo.h (a value of 4K is
//    reasonable).

void fifo_wr(struct fifo *f, unsigned long d);
//    Enqueue the data word d into the FIFO, blocking
//    unless and until the FIFO has room to accept it.
//    Use the semaphore primitives to accomplish blocking and waking.
//    Writing to the FIFO shall cause any and all processes that
//    had been blocked because it was empty to wake up.

unsigned long fifo_rd(struct fifo *f);
//    Dequeue the next data word from the FIFO and return it.
//    Block unless and until there are available words
//    queued in the FIFO.  Reading from the FIFO shall cause
//    any and all processes that had been blocked because it was
//    full to wake up.
