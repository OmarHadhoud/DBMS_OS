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
    //keep waiting for client requesting for sem or releasing sem
    msgrcv(msgqid, &message, sizeof(message) - sizeof(message.mtype), getpid(), !IPC_NOWAIT);
    if (message.acquire_sem == 1)
    {
        acquire_sem(&sem, message.sender);
        message.mtype = message.sender;
        message.sender = getpid();
        //respond to the request (the client is waiting)
        if (msgsnd(msgqid, &message, sizeof(message) - sizeof(message.mtype), !IPC_NOWAIT) == -1)
            perror("Errror in send");
    }
    else if (message.acquire_sem == 0)
        release_sem(&sem, message.sender);
}
/*
 * Requests the semaphor to the query logger
 */
void acquire_query_logger_sem()
{
    struct MsgBuff message = {.mtype = sys_info.query_logger_pid, .sender = getpid(), .acquire_sem = 1};
    //send request to produce
    if (msgsnd(msgqid, &message, sizeof(message) - sizeof(message.mtype), !IPC_NOWAIT) == -1)
        perror("Errror in send");
    //wait for acceptance
    if (msgrcv(msgqid, &message, sizeof(message) - sizeof(message.mtype), getpid(), !IPC_NOWAIT) == -1)
        perror("Errror in receive");
    //entered critical section, print in query logger file as user wants
}
/*
 * Releases the semaphor of the query logger
 */
void release_query_logger_sem()
{
    struct MsgBuff message = {.mtype = sys_info.query_logger_pid, .sender = getpid(), .acquire_sem = 0};
    //release the sem
    if (msgsnd(msgqid, &message, sizeof(message) - sizeof(message.mtype), !IPC_NOWAIT) == -1)
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
}

/*
 * The main of the query logger process
 */
void query_logger_main()
{
    signal(SIGUSR1, query_logger_handler);
    query_logger_on = 1;

    //Initialize Semaphore
    sem_initialize(&sem);

    while (query_logger_on)
        query_log_sem_controller();

    sem_delete(&sem);
    printf("Query Logger dying bye bye xd\n");
}