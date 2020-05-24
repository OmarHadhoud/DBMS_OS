#include "parent.h"

//To be able to open config file
#include <stdio.h>
//To allocate and free memory we need stdlib.
#include <stdlib.h>
//To use process related functions.
#include <sys/types.h>
#include <unistd.h>

/*
 * Parent functions.
 */

/*
 *  Reads the configuration from the config.txt file.
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
    N = fscanf(fptr, "%d", &N); //Read the number of clients from configuration file
    return N;
}

/*
 *  Fork the processes.
 */
void fork_children(int n)
{
    //Allocate memory for array of pids for parent process
    pids = (pid_t*) malloc(n*sizeof(pid_t));
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
            break;
        }
        pids[i] = pid;
    }
    process_role = parent;
}

/*
 * //TODO:
 *  Initialize resources.
 */
void initialize_resources()
{
    
}

/*
 * //TODO:
 *  Send for every process its role and resources it need.
 */
void setup_processes()
{
    
}

/*
 * //TODO:
 *  Receives role and resources for all forked processes.
 */
void receive_setup()
{
    
}


