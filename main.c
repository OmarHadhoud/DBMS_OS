#include "parent.h"


void main()
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
    if(process_role == parent)
        parent_main();
    else if(process_role == db_manager)
        db_manager_main();
    else if(process_role == db_client)
        db_client_main();
    else if(process_role == logger)
        logger_main();
    else if(process_role == query_logger)
        query_logger_main();
    else if(process_role == deadlock_detector)
        deadlock_detector_main();   
       
}