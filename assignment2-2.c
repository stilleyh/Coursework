#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>

/*
This program provides a possible solution for first readers writers problem using mutex and semaphore.
I have used 10 readers and 5 producers to demonstrate the solution. You can always play with these values.
*/

#define MAXRDR 5

sem_t wrt;
sem_t rdr;
pthread_mutex_t mutex;
int cnt = 1;
int numreader = 0;

// Counting wrapper for BSems
typedef struct {
    int val;
    sem_t gate;
    sem_t mutex;
} CSem;


void *writer(void *wno, CSem* csem)
{   
    sem_wait(&wrt);
    cnt = cnt*2;
    printf("Writer %d modified cnt to %d\n",(*((int *)wno)),cnt);
    sem_post(&wrt);
    return NULL;
}
void *reader(void *rno, CSem* csem)
{   
    sem_wait(&rdr); // Add reader to critical section
    // Reader acquire the lock before modifying numreader
    pthread_mutex_lock(&mutex);

    numreader++;

    if(numreader == 1) {
        sem_wait(&wrt); // If this id the first reader, then it will block the writer
    }
    pthread_mutex_unlock(&mutex);
    // Reading Section
    printf("Reader %d: read cnt as %d\n",*((int *)rno),cnt);

    // Reader acquire the lock before modifying numreader
    pthread_mutex_lock(&mutex);
    numreader--;
    if(numreader == 0) {
        sem_post(&wrt); // If this is the last reader, it will wake up the writer.
    }

    pthread_mutex_unlock(&mutex);
    sem_post(&rdr);
    return NULL;
}


// 

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

    pthread_t read[10],write[5];
    csem->val = K;
    sem_init(&csem->gate, 0, (K > 0 ? 1 : 0)); // 1 = open
    sem_init(&csem->mutex, 0, 1);  // " "

    int a[10] = {1,2,3,4,5,6,7,8,9,10}; // Just used for numbering the producer and consumer

    for(int i = 0; i < 10; i++) {
        pthread_create(&read[i], NULL, (void *)reader, (void *)&a[i]);
    }
    for(int i = 0; i < 5; i++) {
        pthread_create(&write[i], NULL, (void *)writer, (void *)&a[i]);
    }

    for(int i = 0; i < 10; i++) {
        pthread_join(read[i], NULL);
    }
    for(int i = 0; i < 5; i++) {
        pthread_join(write[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    sem_destroy(&wrt);
    sem_destroy(&rdr);

    return 0;
    
}