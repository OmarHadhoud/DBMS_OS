#include "manager.h"
#include <sys/types.h>
#include <sys/msg.h>
//To use shared memory
#include <sys/ipc.h>
#include <sys/shm.h>
//To use signals
#include <signal.h>
#include <string.h>
//To allocate and free memory we need stdlib.
#include <stdlib.h>

int current_key = 0;

/*
 * Manager functions.
 */
void manager_handler(int signum)
{
    if (signum==SIGUSR1) manager_On=0;
    printf("Parent terminating manager..\n");
    struct MsgBuff message={.mtype = sys_info.db_manager_pid , .sender = 0 , .action = NOTIFY};
    if (msgsnd(sys_info.logger_msgqid, &message, sizeof(message)-sizeof(message.mtype), !IPC_NOWAIT)==-1) perror("Errror in send");
}


/*
 * The main of the manager process
 */
void manager_main()
{
    //printf("I'm the manager, my pid is : %d\n", getpid());
    manager_shared_memory = (struct ManagerSharedMemory *)shmat(sys_info.records_shmid, NULL, 0);
    //printf("size of struct= %d\n",sizeof(struct ManagerSharedMemory));
    setup_records();
    signal(SIGUSR1, manager_handler);
    manager_On = 1;
    while (manager_On)
    {
        check_operation();
    }
    for (int i = 0; i <= current_key; i++)
    {
        sem_delete(manager_shared_memory->records[i].sem1);
        free(manager_shared_memory->records[i].sem1);
    }
    shmdt(manager_shared_memory);
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
    printf("salary = %d \n", salary);
    printf("name is %s \n", name);
    // printf("current_key = %d \n",current_key);
    //if(manager_shm_record[current_key])
    manager_shm_record[current_key].key = current_key;

    //manager_shm_record[current_key].salary = buff.message_record.salary;

    manager_shm_record[current_key].salary = salary;

    for (int i = 0; i <= 20; i++)
    {
        if (name[i] >= 'a' && name[i] <= 'z')
            manager_shm_record[current_key].name[i] = name[i];
    }

    manager_shm_record[current_key].sem1 = malloc(sizeof(struct Sem));
    sem_initialize(manager_shm_record[current_key].sem1);
    struct message buff;

    //return key to client
    buff.message_record.key = current_key++;
    //buff.mtype = buff.pid;
    buff.mtype = pid;
    printf("I added from client %d\n", pid);
    int send_val = msgsnd(sys_info.dbmanager_msgqid, &buff, sizeof(buff.message_record), !IPC_NOWAIT);

    if (send_val == -1)
        perror("Error in send");
}

/*
 * add or subtract a certain value to the salary of a certain record.
 */
void manager_modify(int key, int value)
{
    if (key > current_key)
    {
        printf("the record have not initialized yet");
    }
    else
    {
        printf("I will modify salary with= %d of record %d \n", value, key);
        struct record *manager_shm_record = manager_shared_memory->records;
        manager_shm_record[key].salary += value;
    }
}

/*
 * lock and return the lock, if locked add in queue.
 */
void manager_acquire(int key, int pid)
{
    if (key > current_key)
    {
        printf("the record have not initialized yet");
    }
    else
    {
        struct record *manager_shm_record = manager_shared_memory->records;
        printf("I will acquire sem now key = %d and pid =%d \n", key, pid);
        if (&manager_shm_record[key].sem1)
            acquire_sem(manager_shm_record[key].sem1, pid);
        struct message buff;
        buff.mtype = pid;
        int send_val = msgsnd(sys_info.dbmanager_msgqid, &buff, sizeof(buff), !IPC_NOWAIT);
        if (send_val == -1)
            perror("Error in send");
    }
}

/*
 * removes the lock, grants it to next in queue.
 */
void manager_release(int key, int pid)
{
    if (key > current_key)
    {
        printf("the record have not initialized yet");
    }
    else
    {
        struct record *manager_shm_record = manager_shared_memory->records;
        printf("I will release sem now \n");
        release_sem(manager_shm_record[key].sem1, pid);
    }
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
            //  Produce("the manager add record and return the key to client");
            manager_add_record(buff.message_record.name, buff.message_record.salary, buff.pid);
        }
        else if (type == 2)
        {
            //Produce("the manager add record and return the key to client");
            manager_modify(buff.message_record.key, buff.message_record.salary);
        }
        else if (type == 3)
        {
            manager_acquire(buff.message_record.key, buff.pid);
        }
        else if (type == 4)
        {

            manager_release(buff.message_record.key, buff.pid);
        }
    }
}

void setup_records()
{
    for (int i = 0; i < 1000; i++)
        manager_shared_memory->records[i].key = -1;
    current_key = 0;
}