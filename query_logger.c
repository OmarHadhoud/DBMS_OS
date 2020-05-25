#include <stdio.h>
#include <unistd.h>

/*
 * The main of the query_logger process
 */
void query_logger_main()
{
    printf("I'm the query_logger, my pid is : %d\n", getpid());
}