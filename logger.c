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
 * If a message is recived filter its type and act accordingly
 * Mainly handles giving other processes the semaphore and giving them the acceptace message if it's their turn
 */
void MsgSystem(){
    struct MsgBuff message={.mtype = sys_info.logger_pid , .sender = getpid() , .action = ACQUIRE};
    if(logger_shared_memory->producer_idx-logger_shared_memory->consumer_idx!=0){ //if there is a need to consume don't wait 
        if (msgrcv(sys_info.logger_msgqid, &message, sizeof(message)-sizeof(message.mtype), getpid(), IPC_NOWAIT)==-1) 
        {
            return; //return if no message received
        }
    } else  {//keep waiting for new produced if all is consumed
        if (msgrcv(sys_info.logger_msgqid, &message, sizeof(message)-sizeof(message.mtype), getpid(), !IPC_NOWAIT)==-1 ) if(loggerOn) perror("Errror in receive");
    }
    if (message.action==ACQUIRE) 
    {
        if((logger_shared_memory->producer_idx+1)%MEM_SIZE == logger_shared_memory->consumer_idx%MEM_SIZE ) Consume(); //if the queue is full consume before giving sem

        acquire_sem(&sem,message.sender);
        if(*(sem.sem_holder)==message.sender){ //if sem is accquired give response
            message.mtype=message.sender;
            message.sender=getpid();
            //respond to the request (the client is waiting)
            if (msgsnd(sys_info.logger_msgqid, &message, sizeof(message)-sizeof(message.mtype), !IPC_NOWAIT)==-1) perror("Errror in send");
        }
    
    }else if(message.action==RELEASE)
    {
        release_sem(&sem,message.sender);
        if(*(sem.sem_holder)!=-1){//give sem to next in queue
            message.mtype=*(sem.sem_holder);
            message.sender=getpid();
            //respond to the request (the client is waiting)
            if (msgsnd(sys_info.logger_msgqid, &message, sizeof(message)-sizeof(message.mtype), !IPC_NOWAIT)==-1) perror("Errror in send");
        }
    }
}
/*
 * This is meant to be used in other processes   
 * Sends a char* to the logger to log
 * and waits for a acceptance message to access the shared memory
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
}
/*
 *  Finds logs sent by other processes and logs them in their corresponding file
 *  Returns 1 if there is nothing to consume, 0 otherwise
 */
int Consume(){
    struct SingleLog log;
    if ((logger_shared_memory->producer_idx%MEM_SIZE == logger_shared_memory->consumer_idx%MEM_SIZE ))
    {     
        if(forked_pid==0) kill(getpid(), SIGSTOP); //if I am the sub process raise sigstop
        return 1;
    } //no logs to consume
    
    //get the current time and format it in time_buffer
    time_t timer;
    char time_buffer[26];
    struct tm* tm_info;
    timer = time(NULL);
    tm_info = localtime(&timer);
    strftime(time_buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);


    // adding logs
    log = logger_shared_memory->logs_array[(logger_shared_memory->consumer_idx+1)%MEM_SIZE];
    char f_name[FILE_NAME_MAX];
    snprintf(f_name, FILE_NAME_MAX,"%d.txt", (int)log.client_pid);
    FILE *F = fopen(f_name, "a");
    if (F == NULL) perror("Couldn't create file");

    if(L_VERBOS) printf("--------------Starting consuming %d  %d _%d \n ",logger_shared_memory->consumer_idx, logger_shared_memory->producer_idx,getpid());
    
    fprintf(F, "%s: %s\n",time_buffer ,log.msg);
    fflush(F);
    fclose(F);
    logger_shared_memory->consumer_idx++;
    return 0;                       
}

/*
 * terminate the logger when getting SIGUSR1
 */
void handler(int signum)
{
	if (signum==SIGUSR1) loggerOn=0;
    printf("Parent terminating logger..\n");
    if(forked_pid>0) {
        kill(forked_pid, SIGTERM); //terminate the forked process 
        printf("logger terminated forked process!\n");
    }
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
    loggerOn=1;
    forked_pid=-1;
    //Attach to logger shared memory
    logger_shared_memory = (struct LoggerSharedMemory*) shmat(sys_info.logger_shmid,NULL,0);
    logger_shared_memory->consumer_idx=0;
    logger_shared_memory->producer_idx=0;
    //Initialize Semaphore
    sem_initialize(&sem);

    //Fork a process to handle the consuming
    if(L_FORK) {
        forked_pid = fork();
        while(loggerOn){
            if(forked_pid == -1 && L_FORK)
            {
                perror("Couldn't fork all the processes!");
                //accept 1 produced then consume 1
                MsgSystem();
                Consume();
            }
            if(forked_pid == 0) //Forked process to consume only
            {
                Consume();
            }else{ //main process handles the messages (semaphores)
                MsgSystem();
                if ((logger_shared_memory->producer_idx%MEM_SIZE == logger_shared_memory->consumer_idx%MEM_SIZE ))
                    kill(forked_pid, SIGSTOP); //pause the consumer nothing to consume
                else
                    kill(forked_pid, SIGCONT); //resume the consumer
            }    
        }
    }else while (loggerOn)
    {
        MsgSystem();
        Consume();
    }
    sem_delete(&sem);
    //Detach from logger shared memory  
    shmdt(logger_shared_memory); 
    printf("Logger dying bye bye\n");

}