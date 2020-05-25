#include <stdio.h>
#include <unistd.h>

/*
 * The main of the manager process
 */
void manager_main()
{
    printf("I'm the manager, my pid is : %d\n", getpid());
}