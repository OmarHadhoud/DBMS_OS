#include "manager.h"
#include <sys/types.h>
#include <sys/msg.h>
//To use shared memory
#include <sys/ipc.h>
#include <sys/shm.h>
//To use signals
#include <signal.h>
#include <string.h>

int current_key = 0;

/*
 * Manager functions.
 */

/*
 * The main of the manager process
 */
void manager_main()
{
    //printf("I'm the manager, my pid is : %d\n", getpid());
    manager_shared_memory = (struct ManagerSharedMemory *)shmat(sys_info.records_shmid, NULL, 0);
    //printf("size of struct= %d\n",sizeof(struct ManagerSharedMemory));
    setup_records();
    while (1)
    {
        check_operation();
    }
}

/*
 * recieves message and ignores key, assign key to record, return key to client.
 */
void manager_add_record(char name[20], int salary, int pid)
{
    // struct message buff;
    struct record *manager_shm_record = manager_shared_memory->records;
    //  int rec_val = msgrcv(sys_info.dbmanager_msgqid, &buff, sizeof(buff.message_record),sys_info.db_manager_pid, IPC_NOWAIT);
    // if (rec_val == -1)
    //  perror("Error in recieve");
    printf("salary = %d - \n", salary);
    printf("name is %s \n", name);
    // printf("current_key = %d \n",current_key);
    //if(manager_shm_record[current_key])
    manager_shm_record[current_key].key = current_key;
    //manager_shm_record[current_key].salary = buff.message_record.salary;

    manager_shm_record[current_key].salary = salary;

    strcpy(manager_shm_record[current_key].name, name);

    sem_initialize(&manager_shm_record[current_key].sem1);
    printf("manger add\n");
    struct message buff;
    //return key to client
    buff.message_record.key = current_key++;
    //buff.mtype = buff.pid;
    buff.mtype = pid;
    printf("I added from client %d\n", pid);
    int send_val = msgsnd(sys_info.dbmanager_msgqid, &buff, sizeof(buff.message_record), 0);
    if (send_val == -1)
        perror("Error in send");
}

/*
 * add or subtract a certain value to the salary of a certain record.
 */
void manager_modify()
{
    struct message buff;
    struct record *manager_shm_record = manager_shared_memory->records;
    int rec_val = msgrcv(sys_info.dbmanager_msgqid, &buff, sizeof(buff.message_record), getpid(), 0);
    if (rec_val == -1)
        perror("Error in recieve");

    manager_shm_record[buff.message_record.key].salary += buff.message_record.salary;
    kill(buff.pid, SIGCONT);
    raise(SIGSTOP);
}

/*
 * lock and return the lock, if locked add in queue.
 */
void manager_acquire()
{
    struct message buff;
    struct record *manager_shm_record = manager_shared_memory->records;
    int rec_val = msgrcv(sys_info.dbmanager_msgqid, &buff, sizeof(buff.message_record), getpid(), IPC_NOWAIT);
    if (rec_val == -1)
        perror("Error in recieve");

    acquire_sem(manager_shm_record[buff.message_record.key].sem1, buff.pid);
    raise(SIGSTOP);
}

/*
 * removes the lock, grants it to next in queue.
 */
void manager_release()
{
    struct message buff;
    struct record *manager_shm_record = manager_shared_memory->records;
    int rec_val = msgrcv(sys_info.dbmanager_msgqid, &buff, sizeof(buff.message_record), getpid(), 0);
    if (rec_val == -1)
        perror("Error in recieve");

    release_sem(manager_shm_record[buff.message_record.key].sem1, buff.pid);
    kill(buff.pid, SIGCONT);
    raise(SIGSTOP);
}
void check_operation()
{

    struct message buff;
    int rec_val = msgrcv(sys_info.dbmanager_msgqid, &buff, sizeof(buff), sys_info.db_manager_pid, !IPC_NOWAIT);
    if (rec_val == -1)
        perror("Error in recieve");

    else
    {
        int type = buff.type_operation;
        if (type == 1)
        {
            printf("call add---------------------%d\n", getpid());
            manager_add_record(buff.message_record.name, buff.message_record.salary, buff.pid);
        }
        else if (type == 2)
        {
        }
        else if (type == 3)
        {
            /* code */
        }
        else
        {
            /* code */
        }
    }
}

void setup_records()
{
    for (int i = 0; i < 1000; i++)
        manager_shared_memory->records[i].key = -1;
    current_key = 0;
}