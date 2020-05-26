#include "logger.h"

#include <stdio.h>
#include <unistd.h>
//To use shared memory
#include <sys/ipc.h>
#include <sys/shm.h>
//To use signals
#include <signal.h>

/*
 * Handles sending and receiving
 */
void MsgSystem(){
    struct MsgBuff message={.mtype = sys_info.logger_pid , .sender = getpid() , .acquire_sem = 1};
    if (msgrcv(msgqid, &message, sizeof(message)-sizeof(message.mtype), getpid(), IPC_NOWAIT)==-1) return; //return if no message received

    if (message.acquire_sem)
    {
        acquire_sem(&sem,message.sender);
        message.mtype=message.sender;
        message.sender=getpid();
        if (msgsnd(msgqid, &message, sizeof(message)-sizeof(message.mtype), !IPC_NOWAIT)==-1) perror("Errror in send");
    
    }else
    {
        release_sem(&sem,message.sender);
            
    }
}
/*
 * Sends a char* to the logger to log
 */
void Produce(char *msg){
    struct SingleLog log;
    log.client_pid = getpid();
    strcpy(log.msg, msg);
    struct MsgBuff message={.mtype = sys_info.logger_pid , .sender = getpid() , .acquire_sem = 1};
    //send request to produce
    printf("Sending message to produce (logger) _%d\n", getpid());
    if (msgsnd(msgqid, &message, sizeof(message)-sizeof(message.mtype), !IPC_NOWAIT)==-1) perror("Errror in send");
    //wait for acceptance 
    printf("Waiting for message to confirm (logger) _%d\n", getpid());
    if (msgrcv(msgqid, &message, sizeof(message)-sizeof(message.mtype), getpid(), !IPC_NOWAIT)==-1) perror("Errror in receive");
    printf("Confirmation received (logger) _%d\n", getpid());
    //entered critical section check if the queue is full
    logger_shared_memory->waiting_pid=getpid();
    printf(" %d_%d\n", (logger_shared_memory->producer_idx+1)%MEM_SIZE , logger_shared_memory->consumer_idx%MEM_SIZE );
    
    while ((logger_shared_memory->producer_idx+1)%MEM_SIZE == logger_shared_memory->consumer_idx%MEM_SIZE )
    {} //queue full please sleep
    printf("Queue isn't full (logger) _%d\n", getpid());
    // adding logs
    logger_shared_memory->logs_array[(logger_shared_memory->producer_idx+1)%MEM_SIZE] = log;
    logger_shared_memory->producer_idx++;

    message.mtype = sys_info.logger_pid ;
    message.sender = getpid();
    message.acquire_sem = 0;

    //release the sem
    if (msgsnd(msgqid, &message, sizeof(message)-sizeof(message.mtype), !IPC_NOWAIT)==-1) perror("Errror in send");
    printf("Semaphore released _%d\n", getpid());
}
/*
 *  Finds logs sent by other processes and logs them in their corresponding file
 */
int Consume(){
    struct SingleLog log;
    if ((logger_shared_memory->producer_idx%MEM_SIZE == logger_shared_memory->consumer_idx%MEM_SIZE ))
    {return 1;} //no logs to consume

    // adding logs
    log = logger_shared_memory->logs_array[(logger_shared_memory->consumer_idx+1)%MEM_SIZE];
    logger_shared_memory->consumer_idx++;
    char * f_name;
    snprintf(f_name, FILE_NAME_MAX, "%d.txt", log.client_pid);
    FILE *F = fopen(f_name, "w");
    if (F == NULL) perror("Couldn't create file");
    fprintf(F, "%s\n", log.msg);
    return 0;                       
}

void handler(int signum)
{
	if (signum==SIGUSR1) loggerOn=0;
}

/*
 * The main of the logger process
 */
void logger_main()
{
    printf("I'm the logger, my pid is : %d\n", getpid());
    signal(SIGUSR1, handler);
    loggerOn =1;
    //Attach to logger shared memory
    logger_shared_memory = (struct LoggerSharedMemory*) shmat(sys_info.logger_shmid,NULL,0);
    logger_shared_memory->consumer_idx=0;
    logger_shared_memory->producer_idx=0;
    //Initialize Semaphore
    sem_initialize(&sem);

    while (loggerOn)
    {
        MsgSystem();
        Consume();
    }
    
    sem_delete(&sem);
    //Detach from logger shared memory  
    shmdt(logger_shared_memory); 

}