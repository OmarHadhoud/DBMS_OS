#include "client.h"



/*
 * Client functions.
 */

/*
 * The main of the client process
 */
void client_main()
{
    printf("I'm the client, my pid is : %d\n", getpid());
    //Get the db shared memory as read-only for queries
    client_shm_record = (struct record*) shmat(sys_info.records_shmid,NULL,SHM_RDONLY);
    char *msg="help me please";
    char *msg2="help me more";
    logger_shared_memory = (struct LoggerSharedMemory*) shmat(sys_info.logger_shmid,NULL,0);
    Produce(msg);
    Produce(msg2);
    acquire_query_logger_sem();
    release_query_logger_sem();
    //Detach shared memory segments
    shmdt(client_shm_record);
    shmdt(logger_shared_memory);
}

/*
 * sends message to the manager, recieves the key back.
 */
void client_add_record(char name[20], int salary, int key)
{
    struct message buff;
    buff.mtype = sys_info.db_manager_pid;
    buff.message_record.key = key;
    buff.message_record.salary = salary;
    buff.pid = getpid();
    for(int i = 0; i<=20; i++)
    {
        if(name[i] >= 'a' && name[i] <= 'z')
            buff.message_record.name[i] = name[i];
    }
    int send_val = msgsnd(sys_info.dbmanager_msgqid, &buff, sizeof(buff.message_record), 0);
    if (send_val == -1)
        perror("Error in send");
        
    kill(sys_info.db_manager_pid,SIGCONT);
    raise(SIGSTOP);

    int rec_val = msgrcv(sys_info.dbmanager_msgqid, &buff, sizeof(buff.message_record), getpid(), 0);
    if (rec_val == -1)
        perror("Error in recieve");

    key = buff.message_record.key;

}

/*
 * Tell the manager, that there is a record with the certain key, I want to add/sub this value to this salary.
 */
void client_modify(int key, int value)
{
    struct message buff;
    buff.mtype = sys_info.db_manager_pid;
    buff.message_record.key = key;
    buff.message_record.salary = value;
    buff.pid = getpid();
    int send_val = msgsnd(sys_info.dbmanager_msgqid, &buff, sizeof(buff.message_record), 0);
    if (send_val == -1)
        perror("Error in send");

    kill(sys_info.db_manager_pid,SIGCONT);
    raise(SIGSTOP);
}

/*
 * Tell the manager I want to lock a certain record.
 */
void client_acquire(int key)
{
    struct message buff;
    buff.mtype = sys_info.db_manager_pid;
    buff.message_record.key = key;
    buff.pid = getpid();
    int send_val = msgsnd(sys_info.dbmanager_msgqid, &buff, sizeof(buff.message_record), 0);
    if (send_val == -1)
        perror("Error in send");

    kill(sys_info.db_manager_pid,SIGCONT);
    raise(SIGSTOP);    
}

/*
 * Send a key to the manager in a message, release it’s key.
 */
void client_release(int key)
{
   struct message buff;
    buff.mtype = sys_info.db_manager_pid;
    buff.message_record.key = key;
    buff.pid = getpid();
    int send_val = msgsnd(sys_info.dbmanager_msgqid, &buff, sizeof(buff.message_record), 0);
    if (send_val == -1)
        perror("Error in send");

    kill(sys_info.db_manager_pid,SIGCONT); 
    raise(SIGSTOP);
}
