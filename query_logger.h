#ifndef _QUERY_LOGGER_H_
#define _QUERY_LOGGER_H_

#include "parent.h"
#include "sem.h"
//To use pid_t
#include <sys/types.h>
//The semaphore of the query logger file
struct Sem query_sem;
//Boolean indicating if query logger is on
int query_logger_on; 

/*
 * Query logger functions.
 * 
 * query_log_sem_controller: Waits for messages requesting/releasing the semaphore. 
 * acquire_query_logger_sem: Acquires the sem.
 * release_query_logger_sem: Releases the sem.
 * query_logger_main: The main of the query logger process.
 */

void query_log_sem_controller();
void acquire_query_logger_sem();
void release_query_logger_sem();

void query_logger_main();
#endif 