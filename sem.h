#ifndef _SEM_H_
#define _SEM_H_
#include "parent.h"
//Including the queue header needed for waiting queue.
#include "queue.h"

#include <sys/types.h>
#include <unistd.h>

#define SEM_VERBOS 0 
//Definition of semaphore

/*
 * Semaphore structure.  
 */
struct Sem {
    _Bool locked; //If locked, no process can acquire this semaphore until it is released.
    pid_t *sem_holder; //The id of the process that holds the semaphore. Should equal NULL if sem is not locked.
    struct Queue *waiting_queue; //The queue of ids of the processes waiting on this semaphore to be released. It is a queue [FIFO].
};

/*struct MsgBuff
{
    //message receiver
	long mtype;
	pid_t sender;
    //wants to acquire_sem or release it (1,0)
    int acquire_sem;
};*/

/*
 * Semaphore functions.
 * 
 * sem_initialize: Initializes the semaphore for N clients.
 * sem_delete: Deletes the semaphore with the arrays created for it.
 *  
 * acquire_sem: The passed process will try to acquire the semaphore,
 * and will get blocked if semaphore is already locked.
 * 
 * release_sem: The passed process will try to release the semaphore,
 * this can only happen if it's the one holding the semaphore. It will*
 * give the semaphore to the next process waiting on the sem queue.
 */

void sem_initialize(struct Sem *s);
void sem_delete(struct Sem *s);
void acquire_sem_msg(struct Sem *s, int pid);
void acquire_sem(struct Sem *s, int pid);
void release_sem(struct Sem *s, int pid);


#endif /* _SEM_H_ */