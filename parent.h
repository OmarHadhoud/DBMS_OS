#ifndef _PARENT_H_
#define _PARENT_H_

//Including the different processes header files.
#include "manager.h"
#include "client.h"
#include "logger.h"
#include "query_logger.h"
#include "deadlock_detector.h"

//Definition of parent process

/*
 * Process variables.  
 */

//Enum for roles of processes
enum role {parent, db_manager, db_client, logger, query_logger, deadlock_detector};
enum role process_role = -1; //Undefined value
//The pids of the processes forked.
int *pids;

/*
 * Parent process functions.
 * 
 * read_config: Reads the configuration file and returns number of clients.
 * fork_children: Forks the N processes needed for the system
 * initialize_resources: Initializes the resources needed for the system.
 * setup_processes: Sends the roles and resources needed for the children to start the system.
 * receive_setup: Receives the setup data (roles and resources) for forked processes.
 */

int read_config();
void fork_children(int n);
void initialize_resources();
void setup_processes();
void receive_setup();

#endif /* _PARENT_H_ */