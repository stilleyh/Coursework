#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

/*
This program provides an implementation of a
Counting Semaphore using Binary Semaphores.
*/

#define K 5


typedef struct { // Counting wrapper for BSems
    int val;
    sem_t gate;
    sem_t mutex;
} CSem;

/*
void Pc(): Add entity to sockets[K] and decrement K
void Vc(): Remove entity from sockets[K] and increment K
*/
void Pc(CSem* csem) { 
    sem_wait(&csem->gate);
    sem_wait(&csem->mutex);
    csem->val--;
    if (csem->val > 0) {
        sem_post(&csem->gate);
    }
    sem_post(&csem->mutex);
}

void Vc(CSem* csem) {
    sem_wait(&csem->mutex);
    csem->val++;
    if (csem->val == 1) {
        sem_post(&csem->gate);
    }
    sem_post(&csem->mutex);
}

int main()
{
    CSem *csem = malloc(sizeof(CSem));

    csem->val = K;
    sem_init(&csem->gate, 0, (K > 0 ? 1 : 0)); // 1 = open
    sem_init(&csem->mutex, 0, 1);  // " "


    sem_destroy(&csem->gate);
    sem_destroy(&csem->mutex);
    free(csem);
    return 0;
}