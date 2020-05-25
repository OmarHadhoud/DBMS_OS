#include <stdio.h>
#include <unistd.h>

/*
 * The main of the logger process
 */
void logger_main()
{
    printf("I'm the logger, my pid is : %d\n", getpid());
}