#include "client.h"
#include <stdio.h>
#include <unistd.h>
//To use shared memory
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
/*
 * The main of the client process
 */
void client_main()
{
    printf("I'm the client, my pid is : %d\n", getpid());
    char *msg="help me please";
    char *msg2="help me more";
    logger_shared_memory = (struct LoggerSharedMemory*) shmat(sys_info.logger_shmid,NULL,0);
    Produce(msg);
    Produce(msg2);
}