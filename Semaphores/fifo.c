/* 
Ivan Chowdhury
ECE 357: Computer Operating Systems
Fall 2018
FIFO Implementation
*/

#include "fifo.h"

void fifo_init(struct fifo *f) {

    sem_init(&f->s1, 0);    
    sem_init(&f->s2, MYFIFO_BUFSIZ);

    sem_init(&f->srw, 1);   
    
    f->rd = 0; 
    f->wr = 0;
}

void fifo_wr(struct fifo *f, unsigned long d) {
    sem_wait(&f->s2);
    sem_wait(&f->srw); 

    f->FIFObuf[f->wr] = d;
    f->wr = (f->wr + 1) % MYFIFO_BUFSIZ;

    sem_inc(&f->srw);
    sem_inc(&f->s1);
}

unsigned long fifo_rd(struct fifo *f) {
    unsigned long d;

    sem_wait(&f->s1);
    sem_wait(&f->srw); 

    d = f->FIFObuf[f->rd];
    f->rd = (f->rd + 1) % MYFIFO_BUFSIZ;

    sem_inc(&f->srw);
    sem_inc(&f->s2);
    
    return d;
}