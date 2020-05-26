#include "client.h"
#include <stdio.h>
#include <unistd.h>

/*
 * The main of the client process
 */
void client_main()
{
    printf("I'm the client, my pid is : %d\n", getpid());
    char *msg="help me please";
    /*logger_shared_memory = (struct LoggerSharedMemory*) shmat(sys_info.logger_shmid,NULL,0);
    logger_shared_memory->consumer_idx=0; //remove me
    printf("this line won't be reached\n");
    Produce(msg);*/
}