#ifndef _SEM_H_
#define _SEM_H_

//Definition of semaphore

/*
 * Semaphore structure.  
 */
struct sem {
    _Bool locked; //If locked, no process can acquire this semaphore until it is released.
    char *sem_holder; //The id of the process that holds the semaphore. Should equal NULL if sem is not locked.
    //Queue **waiting_queue; //The queue of ids of the processes waiting on this semaphore to be released. It is a queue [FIFO].
}

/*
 * Semaphore functions.
 * 
 * sem_initialize: Initializes the semaphore for N clients.
 * sem_delete: Deletes the semaphore with the arrays created for it.
 *  
 * acquire_sem: The passed process will try to acquire the semaphore, and will get blocked if semaphore is already locked.
 * release_sem: The passed process will try to release the semaphore, this can only happen if it's the one holding the semaphore.
 */

void sem_initialize(struct sem *s);
void sem_delete(struct sem *s);

void acquire_sem(struct sem *s, char *pid);
void release_sem(struct sem *s, char *pid);


#endife /* _SEM_H_ */