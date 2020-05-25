#include "parent.h"
#include <stdio.h>
#include <sys/signal.h>
#include <sys/unistd.h>

int main()
{
    //Read the configuration file first and get number of clients from it.
    int n = read_config();
    //Fork n+3 children, n+4 as there is n clients, 1 manager, 1 logger, 1 deadlock_detector and 1 query logger
    fork_children(n+4);
    if(process_role == parent)
    {
        initialize_resources();
        setup_processes();
    } else
        receive_setup();
    process_main();
    exit(0);
}