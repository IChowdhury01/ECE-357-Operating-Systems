/* 
Ivan Chowdhury
ECE 357: Computer Operating Systems
Fall 2018
Test program for Spinlock
*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sched.h>
#include <errno.h>

#include "spinlock.h"

#define N_PROC 64	// Max number of virtual processors
#define NUMPROC 4	// Number of processes used
#define N_ITER 5000000	// Number of iterations

int my_procnum;

int main(int argc, char *argv[]) {
    // Create shared memory space
    int pid[NUMPROC];
	int *counter = mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    char *lock = mmap(NULL, sizeof(char), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0); 
	
	if (lock == MAP_FAILED) {
		fprintf(stderr, "Error: Failed to allocate shared memory space for lock: %s\n", strerror(errno));
		return -1;
	}
	if (counter == MAP_FAILED) {
		fprintf(stderr, "Error: Failed to allocate shared memory space for counter: %s\n", strerror(errno));
		return -1;
	}	

	*counter = 0;
	*lock = 0;

    printf("Ideal Count: %d\n", (NUMPROC * N_ITER));

    // Spawn processes. Increment integer in shared memory (no mutex protection)
	for (int i = 0; i < NUMPROC; i++) {
        if ((pid[i] = fork()) == -1) {
            fprintf(stderr, "Error: Failed to fork process: %s\n", strerror(errno));
            return -1;
        }
        else if (pid[i] == 0) {
			my_procnum = i;
			for (int j = 0; j < N_ITER; j++)
				*counter += 1;
			return 0;
        }
        else {
            my_procnum = 1;
        }
	}
	sleep(1);
	printf("Count [No Mutex Protection]: %d\n", *counter);

    // Spawn processes. Increment integer in shared memory (with mutex protection)
	*counter = 0;
	for (int i = 0; i < NUMPROC; i++) {
		if ((pid[i] = fork()) == -1) {
            fprintf(stderr, "Error: Failed to fork process: %s\n", strerror(errno));
			return -1;
        }
        else if (pid[i] == 0) {
			my_procnum = i;
			for (int j = 0; j < N_ITER; j++) {
				spin_lock(lock);
				*counter += 1;
				spin_unlock(lock);
			}
			return 0;
        }
        else {
            my_procnum = 1;
        }
	}
	sleep(1);
	printf("Count [Mutex Protection]: %d\n", *counter);

    // Free shared memory space
    if (munmap(lock, sizeof(char)) == -1) {
		fprintf(stderr, "Error: Failed to free shared memory space for lock: %s\n", strerror(errno));
		return -1;
	}	
	if (munmap(counter, sizeof(int)) == -1) {
		fprintf(stderr, "Error: Failed to free shared memory space for counter: %s\n", strerror(errno));
		return -1;
	}

	return 0;
}