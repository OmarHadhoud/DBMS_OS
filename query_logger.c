#include "query_logger.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
//To use signals
#include <signal.h>
#include <string.h>
//To use Msgs
#include <sys/types.h>
#include <sys/msg.h>

/*
 * Handles sempahores management
 */
void query_log_sem_controller()
{
    struct MsgBuff message;
    message.acquire_sem = -1; //Prevent garbage value
    //keep waiting for client requesting for sem or releasing sem
    if (msgrcv(sys_info.query_logger_msgqid, &message, sizeof(message)-sizeof(message.mtype), getpid(), !IPC_NOWAIT)==-1 ) if(query_logger_on) perror("Errror in receive");
    if (message.acquire_sem == 1)
    {
        acquire_sem(&query_sem, message.sender);        
        //Printing in log file of query logger
        char msg[80] = "Process with pid ";
        char pid[10];
        sprintf(pid, "%d", message.sender);
        strcat(msg, pid);
        strcat(msg, " has asked for the Semaphore.\n");
        Produce(msg);
        
        message.mtype = message.sender;
        message.sender = getpid();
        //respond to the request (the client is waiting)
        if (msgsnd(sys_info.query_logger_msgqid, &message, sizeof(message) - sizeof(message.mtype), !IPC_NOWAIT) == -1)
            perror("Errror in send");
    }
    else if (message.acquire_sem == 0)
    {
        release_sem(&query_sem, message.sender);
        //Printing in log file of query logger
        char msg[80] = "Process with pid ";
        char pid[10];
        sprintf(pid, "%d", message.sender);
        strcat(msg, pid);
        strcat(msg, " has released the Semaphore.\n");
        Produce(msg);
    }
}
/*
 * Requests the semaphor to the query logger
 */
void acquire_query_logger_sem()
{
    //Printing in log file of client
    Produce("I'm asking to acquire the query logger semaphore");
    struct MsgBuff message = {.mtype = sys_info.query_logger_pid, .sender = getpid(), .acquire_sem = 1};
    //send request to produce
    if (msgsnd(sys_info.query_logger_msgqid, &message, sizeof(message) - sizeof(message.mtype), !IPC_NOWAIT) == -1)
        perror("Errror in send");
    //wait for acceptance
    if (msgrcv(sys_info.query_logger_msgqid, &message, sizeof(message) - sizeof(message.mtype), getpid(), !IPC_NOWAIT) == -1)
        perror("Errror in receive");
    //entered critical section, print in query logger file as user wants
}
/*
 * Releases the semaphor of the query logger
 */
void release_query_logger_sem()
{
    //Printing in log file of client
    Produce("I'm asking to release the query logger semaphore");
    struct MsgBuff message = {.mtype = sys_info.query_logger_pid, .sender = getpid(), .acquire_sem = 0};
    //release the sem
    if (msgsnd(sys_info.query_logger_msgqid, &message, sizeof(message) - sizeof(message.mtype), !IPC_NOWAIT) == -1)
        perror("Errror in send");
}
/*
 * terminate the logger when getting SIGUSR1
 */
void query_logger_handler(int signum)
{
    if (signum == SIGUSR1)
        query_logger_on = 0;
    printf("Parent terminating query logger..\n");
    struct MsgBuff message={.mtype = sys_info.query_logger_pid , .sender = 0 , .acquire_sem = -1};
    if (msgsnd(sys_info.query_logger_msgqid, &message, sizeof(message)-sizeof(message.mtype), !IPC_NOWAIT)==-1) perror("Errror in send");
}

/*
 * The main of the query logger process
 */
void query_logger_main()
{
    printf("I'm the query logger, my pid is : %d\n", getpid());
    //Getting shared memory for normal logger to log in it
    logger_shared_memory = (struct LoggerSharedMemory*) shmat(sys_info.logger_shmid,NULL,0);
    Produce("Query logger has just started...");

    signal(SIGUSR1, query_logger_handler);
    query_logger_on = 1;

    //Initialize Semaphore
    Produce("Query logger initializing semaphore");
    sem_initialize(&query_sem);
    Produce("Query logger starting its infinite loop");
    while (query_logger_on)
        query_log_sem_controller();

    Produce("Query logger finished infinite loop");

    sem_delete(&query_sem);

    Produce("Query logger dying...");
    
    //Detach logger memory
    shmdt(logger_shared_memory);
}