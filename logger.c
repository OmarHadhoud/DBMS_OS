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
//To use Time
#include <time.h>

/*
 * Handles sending and receiving messages from others
 */
void MsgSystem(){
    struct MsgBuff message={.mtype = sys_info.logger_pid , .sender = getpid() , .action = ACQUIRE};
    if(logger_shared_memory->producer_idx-logger_shared_memory->consumer_idx!=0){ //if there is need to consume don't wait 
        if (msgrcv(sys_info.logger_msgqid, &message, sizeof(message)-sizeof(message.mtype), getpid(), IPC_NOWAIT)==-1) 
        {
            if (current_number_of_produced) if(L_VERBOS) printf("\n Didn't rec %d\n", current_number_of_produced);
            
            return; //return if no message received
        }
        //debugging
        //printf("n-----%d:  %d-%d = %d \n",message.sender,logger_shared_memory->producer_idx,logger_shared_memory->consumer_idx,current_number_of_produced);
    } else  {//keep waiting for new produced if all is consumed
        if (msgrcv(sys_info.logger_msgqid, &message, sizeof(message)-sizeof(message.mtype), getpid(), !IPC_NOWAIT)==-1 ) if(loggerOn) perror("Errror in receive");
        //debugging
        //printf("0-----%d:  %d-%d = %d \n",message.sender,logger_shared_memory->producer_idx,logger_shared_memory->consumer_idx,current_number_of_produced);
    }
    if (message.action==ACQUIRE) 
    {
        if((logger_shared_memory->producer_idx+1)%MEM_SIZE == logger_shared_memory->consumer_idx%MEM_SIZE ) Consume(); //if the queue is full consume before giving sem

        acquire_sem(&sem,message.sender);
        message.mtype=message.sender;
        message.sender=getpid();
        //respond to the request (the client is waiting)
        if (msgsnd(sys_info.logger_msgqid, &message, sizeof(message)-sizeof(message.mtype), !IPC_NOWAIT)==-1) perror("Errror in send");
    
    }else if(message.action==RELEASE)
    {
        //debugging
        //printf("r-----:  %d-%d = %d \n",logger_shared_memory->producer_idx,logger_shared_memory->consumer_idx,current_number_of_produced);

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

    struct MsgBuff message={.mtype = sys_info.logger_pid , .sender = getpid() , .action = ACQUIRE};
    //send request to produce
    if(L_VERBOS) printf("Sending message to produce (logger) _%d\n", getpid());
    if (msgsnd(sys_info.logger_msgqid, &message, sizeof(message)-sizeof(message.mtype), !IPC_NOWAIT)==-1) perror("Errror in send");
    //wait for acceptance 
    if (msgrcv(sys_info.logger_msgqid, &message, sizeof(message)-sizeof(message.mtype), getpid(), !IPC_NOWAIT)==-1 ) perror("Errror in receive");
    if(L_VERBOS) printf("Confirmation received (logger) _%d\n", getpid());
    //entered critical section check if the queue is full
    logger_shared_memory->waiting_pid=getpid();

    //logger checks if the queue is full before giving the sem (no need to recheck)

    // adding logs
    logger_shared_memory->logs_array[(logger_shared_memory->producer_idx+1)%MEM_SIZE].client_pid = log.client_pid;
    strcpy(logger_shared_memory->logs_array[(logger_shared_memory->producer_idx+1)%MEM_SIZE].msg,log.msg);
    logger_shared_memory->producer_idx++;
    
    message.mtype = sys_info.logger_pid ;
    message.sender = getpid();
    message.action = RELEASE;
    //release the sem
    if (msgsnd(sys_info.logger_msgqid, &message, sizeof(message)-sizeof(message.mtype), !IPC_NOWAIT)==-1) perror("Errror in send");
    if(L_VERBOS) printf("Semaphore released _%d\n", getpid());
  //  printf("prod-----:  %d-%d = %d \n",logger_shared_memory->producer_idx,logger_shared_memory->consumer_idx,current_number_of_produced);
}
/*
 *  Finds logs sent by other processes and logs them in their corresponding file
 */
int Consume(){
    struct SingleLog log;
    if ((logger_shared_memory->producer_idx%MEM_SIZE == logger_shared_memory->consumer_idx%MEM_SIZE ))
    {     

        if (current_number_of_produced) if(L_VERBOS) printf("\n Didn't consume %d\n", current_number_of_produced);
        return 1;
    } //no logs to consume
    //printf("cons-----:  %d-%d = %d \n",logger_shared_memory->producer_idx,logger_shared_memory->consumer_idx,current_number_of_produced);
    
    //get the current time and format it in time_buffer
    time_t timer;
    char time_buffer[26];
    struct tm* tm_info;
    timer = time(NULL);
    tm_info = localtime(&timer);
    strftime(time_buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);


    // adding logs
    log = logger_shared_memory->logs_array[(logger_shared_memory->consumer_idx+1)%MEM_SIZE];
    logger_shared_memory->consumer_idx++;
    char f_name[FILE_NAME_MAX];
    snprintf(f_name, FILE_NAME_MAX,"%d.txt", (int)log.client_pid);
    FILE *F = fopen(f_name, "a");
    if (F == NULL) perror("Couldn't create file");

    if(L_VERBOS) printf("--------------Starting consuming %d  %d _%d \n ",logger_shared_memory->consumer_idx, logger_shared_memory->producer_idx,getpid());
    
    fprintf(F, "%s: %s\n",time_buffer ,log.msg);
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
    printf("Parent terminating logger..\n");
    struct MsgBuff message={.mtype = sys_info.logger_pid , .sender = 0 , .action = NOTIFY};
    if (msgsnd(sys_info.logger_msgqid, &message, sizeof(message)-sizeof(message.mtype), !IPC_NOWAIT)==-1) perror("Errror in send");
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
        if(current_number_of_produced)printf("--------p:%d c:%d\n",logger_shared_memory->producer_idx,logger_shared_memory->consumer_idx);
    }
    sem_delete(&sem);
    //Detach from logger shared memory  
    shmdt(logger_shared_memory); 
    printf("Logger dying bye bye\n");

}