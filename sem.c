#include "sem.h"

//To allocate and free memory we need stdlib.
#include <stdlib.h>

/*
 * Semaphores functions.
 */

/*
 *  Initializes the semaphore
 */

void sem_initialize(struct sem *s, int N)
{
    int pid_size = sizeof(pid_t); //The maximum size of the string of process id.
    s->locked = fale;
    s->sem_holder =  malloc(pid_size);
    //If we ran out of memory, return.
    if (s->sem_holder == NULL)
    {
        s = NULL;
        return;
    }
    //Queue linked list s->waiting_queue = malloc(N*pid_size); //Every cell in the array is a process id waiting for queue.
    //If we ran out of memory, free the sem_holder and return.
    if(s->waiting_queue == NULL)
    {
        free s->sem_holder;
        s = NULL;
        return;
    }
}
