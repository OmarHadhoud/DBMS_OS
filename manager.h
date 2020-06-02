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
//To use pid_t
#include <sys/types.h>
//Definition of manager process

/*
 * Process variables and structs.  
 */

struct record
{
    int key;
    char name[20];
    int salary;
    struct Sem *sem1;
};
// struct Sem sem1;
struct message
{
    long mtype;
    struct record message_record;
    int pid;
    int type_operation;
};
struct ManagerSharedMemory
{
    struct record records[1000];
};
struct ManagerSharedMemory *manager_shared_memory;

extern int current_key;

/*
 * manager process functions.
 * 
 * manager_main: initial main function that prints the role.
 * manager_add_record: recieves message and ignores key, assign key to record, return key to client.
 * manager_modify: add or subtract a certain value to the salary of a certain record.
 * manager_acquire: lock and return the lock, if locked add in queue.
 * manager_release: removes the lock, grants it to next in queue.
 * setup_records: Makes key of non used record to equal -1
 */

void manager_main();
void manager_add_record(char name[20], int salary, int pid);
void manager_modify();
void manager_acquire();
void manager_release();
void check_operation();
void setup_records();

#endif /* _MANAGER_H_ */