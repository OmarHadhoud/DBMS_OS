#ifndef _PARENT_H_
#define _PARENT_H_

//Including the different processes header files.
#include "manager.h"
#include "client.h"
#include "logger.h"
#include "query_logger.h"
#include "deadlock_detector.h"
//To use pid_t
#include <sys/types.h>

//Definition of parent process

/*
 * Process variables.  
 */

//Enum for roles of processes
enum role {parent, db_manager, db_client, logger, query_logger, deadlock_detector};
enum role process_role; 
//The pids of the processes forked.
int *pids;
int num_forked; //Number of forked processes
//The client number
int client_number;
//The number of active clients
int active_clients;

//The struct of system resources information needed by the processes
//The resources needed are the records_shmid, logger_shmid,
struct system_information {
    int records_shmid; //The shared memory id of the records table.
    int logger_shmid; //The shared memory id of the logger.
    pid_t db_manager_pid;
    pid_t logger_pid;
    pid_t query_logger_pid;
    pid_t deadlock_detector_pid;
    key_t logger_msgqid; //The message queue id for the logger
    key_t query_logger_msgqid; //The message queue id for the query logger
    key_t dbmanager_msgqid; //The message queue id for the db_manager
};
struct system_information sys_info;


/*
 * Parent process functions.
 * 
 * read_config: Reads the configuration file and returns number of clients.
 * fork_children: Forks the N processes needed for the system
 * initialize_resources: Initializes the resources needed for the system.
 * setup_processes: Sends the roles and resources needed for the children to start the system.
 * receive_setup: Receives the setup data (roles and resources) for forked processes.
 * process_main: The main process depending on the role
 * parent_main: The main of the parent process
 * check_client: Checks if the pid given is a client or not.
 */

int read_config();
void fork_children(int n);
void initialize_resources();
void setup_processes();
void receive_setup();
void process_main();
void parent_main();
_Bool check_client(pid_t pid);

#endif /* _PARENT_H_ */