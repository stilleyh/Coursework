#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/types.h>

#define NUM_READERS 10
#define NUM_WRITERS 5

int num_readers;
void Pc(CSem* csem);
void Vc(CSem* csem);

volatile int done = 0;  // termination detection flag

// Counting Semaphore wrapper
typedef struct {
    int val;
    sem_t gate;
    sem_t mutex;
} CSem;

// Shared memory structure
typedef struct {
    char counter[256];    // critical section
    sem_t mutex;          // protects counter
    sem_t read_sem;       // readers can print after writer writes
    int writers_present;
} Shared;

Shared *shared_mem;

// For graceful termination
void sigint_handler(int sig) {
    done = 1;
    for (int i = 0; i < num_readers; i++) {
        sem_post(&shared_mem->read_sem);
    }
}

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



// Reader thread function
void *reader(void *arg) {
    int id = *((int *)arg);
    while (!done) {
        sem_wait(&shared_mem->read_sem);

        sem_wait(&shared_mem->mutex);
        if (shared_mem->writers_present == 0) {  // last writer finished
            sem_post(&shared_mem->mutex);
            break;  // exit loop
        }

        printf("Reader %d: read counter = %s\n", id, shared_mem->counter);
        sem_post(&shared_mem->mutex);
    }
    return NULL;
}

// Writer process function
void writer_process(int id, CSem *shared_csem, Shared *shared_mem) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        exit(1);
    }

    if (pid == 0) { // child process = writer
        // Enter critical section using counting semaphore
        Pc(shared_csem);

        // Critical section: modify shared counter
        snprintf(shared_mem->counter, 256, "Writer %d wrote this", id);
        printf("Writer %d wrote counter: %s\n", id, shared_mem->counter);

        // Exit critical section
        Vc(shared_csem);

        sem_wait(&shared_mem->mutex);
        shared_mem->writers_present--;
        sem_post(&shared_mem->mutex);
        sem_post(&shared_mem->read_sem);
        exit(0); // terminate child process
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        return 1;
    }
    // Convert arguments from strings to integers
    int num_readers = atoi(argv[1]);
    int num_writers = atoi(argv[2]);

    signal(SIGINT, sigint_handler);
    
    // Allocate shared memory for CSem
    CSem *shared_csem = mmap(NULL, sizeof(CSem),
                        PROT_READ | PROT_WRITE,
                        MAP_SHARED | MAP_ANONYMOUS, -1, 0);


    // Initialize counting semaphore
    shared_csem->val = num_writers;
    sem_init(&shared_csem->gate, 1, (num_readers > 0 ? 1 : 0));
    sem_init(&shared_csem->mutex, 1, 1);


    // Define reader threads
    pthread_t readers[num_readers];   // array for reader threads
    int reader_ids[num_readers];      // IDs to pass to threads


    // Allocate shared memory for critical section
    shared_mem = mmap(NULL, sizeof(Shared),
                        PROT_READ | PROT_WRITE,
                        MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    
    shared_mem->writers_present = num_writers;

    // Initialize shared memory semaphore
    sem_init(&shared_mem->mutex, 1, 1);       // protects critical section
    sem_init(&shared_mem->read_sem, 1, 0);    // allow first reader

    // Launch reader threads
    for (int i = 0; i < num_readers; i++) {
    reader_ids[i] = i + 1;
    pthread_create(&readers[i], NULL, reader, &reader_ids[i]);
    }

    // Launch writer processes
    for (int i = 0; i < num_writers; i++) {
        writer_process(i, shared_csem, shared_mem);
    }

    // Wait for all writer processes
    for (int i = 0; i < num_writers; i++) {
        wait(NULL);
    }

    done = 1;

    // Unblock any waiting readers
    for (int i = 0; i < num_readers; i++) {
    sem_post(&shared_mem->read_sem);
}   
    // Wait for all reader threads
    for (int i = 0; i < num_readers; i++) {
        pthread_join(readers[i], NULL);
    }

    // Clean up semaphores
    sem_destroy(&shared_mem->mutex);
    sem_destroy(&shared_mem->read_sem);

    // Unmap shared memory
    munmap(shared_mem, sizeof(Shared));

    return 0;
}