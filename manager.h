#ifndef _MANAGER_H_
#define _MANAGER_H_


#include "sem.h"
#include "parent.h"

#include <signal.h>
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <stdio.h> 
#include <unistd.h>
#include <sys/mman.h>

//Definition of manager process

/*
 * Process variables and structs.  
 */


struct record{
    int key;
    char name[20];
    int salary;
    struct Sem* sem;
};
struct message{
    long mtype;
    struct record message_record;
    int pid;
};

extern int current_key;
struct record* manager_shm_record[1000];    //array of record pointers to attach in the shared memory.

/*
 * manager process functions.
 * 
 * manager_main: initial main function that prints the role.
 * manager_add_record: recieves message and ignores key, assign key to record, return key to client.
 * manager_modify: add or subtract a certain value to the salary of a certain record.
 * manager_acquire: lock and return the lock, if locked add in queue.
 * manager_release: removes the lock, grants it to next in queue.
 */

void manager_main();
void manager_add_record();
void manager_modify();
void manager_acquire();
void manager_release();


#endif /* _MANAGER_H_ */