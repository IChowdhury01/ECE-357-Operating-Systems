/* 
Ivan Chowdhury
ECE 357: Computer Operating Systems
Fall 2018
FIFO Testing Framework
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/types.h>

#include "fifo.h"

#define N_ITER 1000  // Number of iterations

void handler(int signo) {   // Signal handler for semaphores
}

int main(int argc, char *argv[])
{
    int pid;
    unsigned long d;
    
    // Establish a struct FIFO in shared memory
    struct fifo *f; 
    if ((f = mmap(NULL, sizeof(struct fifo), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0)) == MAP_FAILED) {
        fprintf(stderr, "Error: Failed to allocate shared memory space for FIFO: %s\n", strerror(errno));
        return -1;
    }

    // Set up sigaction struct for semaphores
    struct sigaction sa;
    sa.sa_flags = 0;
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    
    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        fprintf(stderr, "Error: Failed to update signal action: %s\n", strerror(errno));
        return -1;
    }
        
    fifo_init(f);   // Initialize shared memory FIFO

    // Create writer and reader virtual processors
    if ((pid = fork()) == -1) {
        fprintf(stderr, "Error: Failed to fork process into reader and writer processes: %s\n", strerror(errno));
        return -1;
    }
    else if (pid == 0) { // Writer process
        unsigned long i;
        for (i = 0 ; i < N_ITER ; i++ ) {   // Send fixed number of sequentially-numbered data
            fifo_wr(f, i);
        }
        return 0;  
    }
    else { // Reader process
        for (unsigned i = 0 ; i < N_ITER ; i++) {   // Read all data sent by writer process
            d = fifo_rd(f);
        }
    }
    // Test and report results
    if (d == N_ITER - 1) 
        fprintf(stderr, "Success: All of the data sent by the writer process was received by the reader process\n");
    else
        fprintf(stderr, "Failure: Not all of the values sent were received by the FIFO read process\n");

    return 0;
}