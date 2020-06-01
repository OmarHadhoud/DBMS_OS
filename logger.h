#ifndef _LOGGER_H_
#define _LOGGER_H_
#define MEM_SIZE 10
#define FILE_NAME_MAX 20
#define MAX_LOG_LINE_SIZE 100
#define L_VERBOS 0
#include "parent.h"
#include "sem.h"
//To use pid_t
#include <sys/types.h>
struct MsgBuff
{
    //message receiver
	long mtype;
	pid_t sender;
    //wants to acquire_sem or release it (1,0)
    int acquire_sem;
};
struct SingleLog {
    pid_t client_pid;
    char msg[MAX_LOG_LINE_SIZE];
};
struct LoggerSharedMemory {
    unsigned int producer_idx;
    unsigned int consumer_idx;
    pid_t waiting_pid;
    struct SingleLog logs_array[MEM_SIZE];    
};
struct LoggerSharedMemory *logger_shared_memory;
struct Sem sem;
int current_number_of_produced;
int loggerOn; 

void MsgSystem();
void Produce(char *msg);
int Consume();

void logger_main();
#endif 