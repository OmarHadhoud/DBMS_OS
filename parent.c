#include "parent.h"

//To be able to open config file
#include <stdio.h>
//To allocate and free memory we need stdlib.
#include <stdlib.h>
//To use process related functions.
#include <sys/types.h>
#include <unistd.h>
//To use shared memory
#include <sys/ipc.h>
#include <sys/shm.h>
//To send initializing system messages.
#include "messages.h"

/*
 * Parent functions.
 */

/*
 * Reads the configuration from the config.txt file.
 */
int read_config()
{
    //Open config file
    FILE *fptr = fopen("config.txt", "r");
    //If configuration file doesn't exist, exit the application
    if (fptr == NULL)
    {
        perror("Couldn't open the configurationf file");
        exit(-1);
    }
    int N; //Number of clients
    fscanf(fptr, "%d", &N); //Read the number of clients from configuration file
    fclose(fptr); //Close the file when done
    return N;
}

/*
 * Fork the processes.
 */
void fork_children(int n)
{
    num_forked = n;
    //Create the message queue to be passed to forked children to be 
    //able to communicate with them.
    msgqid = msgget(IPC_PRIVATE, 0644);

    //Allocate memory for array of pids for parent process
    pids = (pid_t*) malloc((n+1)*sizeof(pid_t));
    //First index is the parent process
    pids[0] = getpid();

    int pid;
    for(int i = 0; i < n; i++) //Fork n children
    {
        pid = fork();
        if(pid == -1)
        {
            perror("Couldn't fork all the processes!");
            exit(-1);
        }
        if(pid == 0) //If not parent, free pids as it is not needed and break
        {
            free(pids);
            process_role = -1; //Initialize with -1 as role is undefined yet.
            return;
        }
        pids[i+1] = pid;
    }
    process_role = parent;
}

/*
 *  Initialize resources.
 */
void initialize_resources()
{
    //Create the shared memory segment for records struct.
    //TODO: Use sizeof(record_struct) instead when done
    sys_info.records_shmid = shmget(IPC_PRIVATE,sizeof(int),0666|IPC_CREAT);
    
    //Create the shared memory segment for logger struct.
    //TODO: Use sizeof(logger_struct) instead when done
    sys_info.logger_shmid = shmget(IPC_PRIVATE,sizeof(int),0666|IPC_CREAT);

    //The first forked process is the db_manager, second is logger, third is query_logger,
    //Fourth is deadlock_detector, the rest are the db_clients.
    sys_info.db_manager_pid = pids[1];
    sys_info.logger_pid = pids[2];
    sys_info.query_logger_pid = pids[3];
    sys_info.deadlock_detector_pid = pids[4];
}

/*
 * Send for every process its role and resources struct it need.
 */
void setup_processes()
{
    //Create message structs array for every role
    struct initializing_msg setup_msg[num_forked];  
    //Assign the system info ptr in the message
    for(int i = 0; i < num_forked; i++)
        setup_msg[i].sys_info = &sys_info;
    
    //Assign roles to each process
    setup_msg[0].role = db_manager;
    setup_msg[1].role = logger;
    setup_msg[2].role = query_logger;
    setup_msg[3].role = deadlock_detector;
    for(int i = 4; i < num_forked; i++)
        setup_msg[i].role = db_client;

    //Assign the pid to receive message from
    for(int i = 0; i < num_forked; i++)
        setup_msg[i].mtype = pids[i+1];

    //Start sending messages
    int sz = sizeof(setup_msg[0]) - sizeof(setup_msg[0].mtype); //The size of the message
    for(int i = 0; i < num_forked; i++)
    {
        int send_val = msgsnd(msgqid, &setup_msg[i], sz, 0);
        if(send_val == -1)
        {
            perror("Couldn't send initializing messages!");
            exit(-1);
        }
    }
}

/*
 * Receives role and resources for all forked processes.
 */
void receive_setup()
{
    struct initializing_msg msg_buffer; //The buffer to receive message in 
    int sz = sizeof(msg_buffer) - sizeof(msg_buffer.mtype); //The size of the message
    
    //Receive the initialzing message
    int rec_val = msgrcv(msgqid, &msg_buffer, sz, getpid(),0);
    
    //If can't receive message
    if(rec_val == -1)
    {
        perror("COuldn't receive tht initializing message");
        exit(-1);
    }

    //Get the system info
    sys_info = *msg_buffer.sys_info;
    //Get the process role
    process_role = msg_buffer.role;
}

/*
 * The process main() code
 */
void process_main()
{
    //Do the main function depending on the role
    if(process_role == parent)
        parent_main();
    else if(process_role == db_manager)
        manager_main();
    else if(process_role == db_client)
        client_main();
    else if(process_role == logger)
        logger_main();
    else if(process_role == query_logger)
        query_logger_main();
    else if(process_role == deadlock_detector)
        deadlock_detector_main();   
}

/*
 * The main of the parent process
 */
void parent_main()
{
    printf("I'm the parent, my pid is : %d\n", getpid());
}