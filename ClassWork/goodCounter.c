/*
    Simple program that solves a race condition using mutex
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define NUM_THREADS 5
#define MAX_COUNTS 1000000

// Thread function declaration
void * startThread(void * arg);

// Global variables
int counter = 0;
// A global mutex variable
pthread_mutex_t count_lock = PTHREAD_MUTEX_INITIALIZER;

int main()
{
    pthread_t tids[NUM_THREADS];
    int status;
    long result;
    
    // Loop to create the threads
    for (long i=0; i<NUM_THREADS; i++)
    {
        status = pthread_create(&tids[i], NULL, startThread, (void *)i);
        printf("Thread %ld created with ID: %ld\n", i, tids[i]);
        if (status != 0)
        {
            perror("ERROR: pthread_create");
            exit(EXIT_FAILURE);
        }
    }
    
    // Wait for all threads to finish
    for (int i=0; i<NUM_THREADS; i++)
    {
        status = pthread_join(tids[i], (void *)&result);
        if (status != 0)
        {
            perror("ERROR: pthread_join");
            exit(EXIT_FAILURE);
        }
        printf("Thread %d returned value %ld\n", i, result);
    }
    
    printf("Main thread finishing\n");
    printf("The final counter is: %d\n", counter);
    // Wait for all other attached threads to finish
    pthread_exit(NULL);
    
    return 0;
}

void * startThread(void * arg)
{
    long index = (long) arg;
    //int status = pthread_detach(pthread_self());
    
    for (int i=0; i<MAX_COUNTS; i++)
    {
        // Start the critical section
        pthread_mutex_lock(&count_lock);
            counter++;
        // End the critical section
        pthread_mutex_unlock(&count_lock);
    }
    printf("\tThread %ld is about to finish\n", index);
    
    pthread_exit( (void *) (index * 2) );
}