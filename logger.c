#include "logger.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
//To use shared memory
#include <sys/ipc.h>
#include <sys/shm.h>
//To use signals
#include <signal.h>
#include <string.h>
//To use Msgs
#include <sys/types.h>
#include <sys/msg.h>

/*
 * Handles sending and receiving messages from others
 */
void MsgSystem(){
    struct MsgBuff message={.mtype = sys_info.logger_pid , .sender = getpid() , .acquire_sem = 1};
    
    //if(current_number_of_produced) //if there is need to consume don't wait
        if (msgrcv(msgqid, &message, sizeof(message)-sizeof(message.mtype), getpid(), IPC_NOWAIT)==-1) 
        {
            if (current_number_of_produced) if(L_VERBOS) printf("\n Didn't rec %d\n", current_number_of_produced);
            
            return; //return if no message received
        }
    /*else  {//keep waiting for new produced if all is consumed
        if (msgrcv(msgqid, &message, sizeof(message)-sizeof(message.mtype), getpid(), !IPC_NOWAIT)==-1) perror("Errror in receive");
    }*/
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

    kill(sys_info.logger_pid, SIGCONT);
    struct MsgBuff message={.mtype = sys_info.logger_pid , .sender = getpid() , .acquire_sem = 1};
    //send request to produce
    if(L_VERBOS) printf("Sending message to produce (logger) _%d\n", getpid());
    if (msgsnd(msgqid, &message, sizeof(message)-sizeof(message.mtype), !IPC_NOWAIT)==-1) perror("Errror in send");
    //wait for acceptance 
    if (msgrcv(msgqid, &message, sizeof(message)-sizeof(message.mtype), getpid(), !IPC_NOWAIT)==-1) perror("Errror in receive");
    if(L_VERBOS) printf("Confirmation received (logger) _%d\n", getpid());
    //entered critical section check if the queue is full
    logger_shared_memory->waiting_pid=getpid();

    kill(sys_info.logger_pid, SIGCONT);
    while ((logger_shared_memory->producer_idx+1)%MEM_SIZE == logger_shared_memory->consumer_idx%MEM_SIZE )
    {} //queue full please sleep
    // adding logs
    logger_shared_memory->logs_array[(logger_shared_memory->producer_idx+1)%MEM_SIZE].client_pid = log.client_pid;
    strcpy(logger_shared_memory->logs_array[(logger_shared_memory->producer_idx+1)%MEM_SIZE].msg,log.msg);
    logger_shared_memory->producer_idx++;
    
    message.mtype = sys_info.logger_pid ;
    message.sender = getpid();
    message.acquire_sem = 0;
    //release the sem
    if (msgsnd(msgqid, &message, sizeof(message)-sizeof(message.mtype), !IPC_NOWAIT)==-1) perror("Errror in send");
    if(L_VERBOS) printf("Semaphore released _%d\n", getpid());
    kill(sys_info.logger_pid, SIGCONT);
}
/*
 *  Finds logs sent by other processes and logs them in their corresponding file
 */
int Consume(){
    struct SingleLog log;
    if ((logger_shared_memory->producer_idx%MEM_SIZE == logger_shared_memory->consumer_idx%MEM_SIZE ))
    {     
        //kill(getpid(), SIGSTOP);
        if (current_number_of_produced) if(L_VERBOS) printf("\n Didn't consume %d\n", current_number_of_produced);
        return 1;
    } //no logs to consume

    // adding logs
    log = logger_shared_memory->logs_array[(logger_shared_memory->consumer_idx+1)%MEM_SIZE];
    logger_shared_memory->consumer_idx++;
    char f_name[FILE_NAME_MAX];
    snprintf(f_name, FILE_NAME_MAX,"%d.txt", (int)log.client_pid);
    FILE *F = fopen(f_name, "a");
    if (F == NULL) perror("Couldn't create file");

    if(L_VERBOS) printf("--------------Starting consuming %d  %d _%d \n ",logger_shared_memory->consumer_idx, logger_shared_memory->producer_idx,getpid());
    
    fprintf(F, "%s\n", log.msg);
    fflush(F);
    fclose(F);
    return 0;                       
}

/*
 * terminate the logger when getting SIGUSR1
 */
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
        current_number_of_produced=logger_shared_memory->producer_idx- logger_shared_memory->consumer_idx;

    }
    printf("Logger dying bye bye\n");
    sem_delete(&sem);
    //Detach from logger shared memory  
    shmdt(logger_shared_memory); 

}