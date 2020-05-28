#include "manager.h"


int current_key = 0;

/*
 * Manager functions.
 */

/*
 * The main of the manager process
 */
void manager_main()
{
    printf("I'm the manager, my pid is : %d\n", getpid());
    raise(SIGSTOP);
}

/*
 * recieves message and ignores key, assign key to record, return key to client.
 */
void manager_add_record()
{
    struct message buff;
    int rec_val = msgrcv(msgqid, &buff, sizeof(buff.message_record), getpid(), 0);
    if (rec_val == -1)
        perror("Error in recieve");
    
    manager_shm_record[current_key] = (struct record*) shmat(sys_info.records_shmid,(void*)0,0);    //attach to shared memory
    manager_shm_record[current_key] -> key = current_key;
    manager_shm_record[current_key] -> salary = buff.message_record.salary;   
    for(int i = 0; i<=20; i++)
    {
        if(buff.message_record.name[i] >= 'a' && buff.message_record.name[i] <= 'z')
            manager_shm_record[current_key] -> name[i] = buff.message_record.name[i];
    }
    manager_shm_record[current_key] -> sem->locked = 0;

    //return key to client
    buff.message_record.key = current_key++;
    buff.mtype = buff.pid;
    int send_val = msgsnd(msgqid, &buff, sizeof(buff.message_record), 0);
    if (send_val == -1)
        perror("Error in send");
    
    kill(buff.pid,SIGCONT);
    raise(SIGSTOP);
}

/*
 * add or subtract a certain value to the salary of a certain record.
 */
void manager_modify()
{
    struct message buff;
    int rec_val = msgrcv(msgqid, &buff, sizeof(buff.message_record), getpid(), 0);
    if (rec_val == -1)
        perror("Error in recieve");

    manager_shm_record[buff.message_record.key] ->salary += buff.message_record.salary;
    kill(buff.pid,SIGCONT);
    raise(SIGSTOP);
}

/*
 * lock and return the lock, if locked add in queue.
 */
void manager_acquire()
{
    struct message buff;
    int rec_val = msgrcv(msgqid, &buff, sizeof(buff.message_record), getpid(), 0);
    if (rec_val == -1)
        perror("Error in recieve");

    acquire_sem (manager_shm_record[buff.message_record.key] -> sem, buff.pid);
    raise(SIGSTOP);
}

/*
 * removes the lock, grants it to next in queue.
 */
void manager_release()
{
   struct message buff;
    int rec_val = msgrcv(msgqid, &buff, sizeof(buff.message_record), getpid(), 0);
    if (rec_val == -1)
        perror("Error in recieve");

    release_sem (manager_shm_record[buff.message_record.key] -> sem, buff.pid);
    kill(buff.pid,SIGCONT);
    raise(SIGSTOP);
    
}
