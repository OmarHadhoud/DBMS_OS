#include "sem.h"

//To allocate and free memory we need stdlib.
#include <stdlib.h>
//To send signal to process acquiring the semaphore to continue.
#include <signal.h>
/*
 * Semaphores functions.
 */

/*
 *  Initializes the semaphore
 */

void sem_initialize(struct Sem *s)
{
    int pid_size = sizeof(pid_t); //The maximum size of process id.
    s->locked = 0;
    s->sem_holder = malloc(pid_size);
    //If we ran out of memory, return.
    if (s->sem_holder == NULL)
    {
        s = NULL;
        return;
    }
    s->waiting_queue = create_queue();
    //If we ran out of memory, free the sem_holder and return.
    if (s->waiting_queue == NULL)
    {
        free(s->sem_holder);
        s = NULL;
        return;
    }
}

/*
 *  Deletes the semaphore and frees any memory dynamically allocated.
 */
void sem_delete(struct Sem *s)
{
    free(s->sem_holder);
    delete_queue(s->waiting_queue);
}

/*
 *  Acquires the semaphor to the passed process.
 *  It sends a signal to wake the process if it can acquire the semaphore,
 *  if it can't, it will add it to the waiting queue.
 */
void acquire_sem(struct Sem *s, int pid)
{
    if (s->locked == 0)
    {
        //Acquire the sem, wake the process.
        s->locked = 1;
        s->sem_holder = pid;
        kill(pid, SIGCONT);
    }
    else //Add the process to the waiting queue.
        enqueue(s->waiting_queue, pid);
}

/*
 *  Releases the semaphor passed.
 *  Wakes up the next process in waiting queue of this semaphore, if there was any.
 */
void release_sem(struct Sem *s, int pid)
{
    if (s->sem_holder != pid) //Make sure the one releasing sem is the holder.
        return;

    s->locked = 0;
    s->sem_holder = NULL;
    //If no process is in waiting queue, return.
    if (s->waiting_queue->front == NULL)
        return;
    //Get the next process id to acquire the semaphore.
    int next_pid = s->waiting_queue->front;
    dequeue(s->waiting_queue);
    acquire_sem(s, next_pid); //Give the semaphore to the process waiting.
}
