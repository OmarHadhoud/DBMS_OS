#include <stdio.h>
#include <unistd.h>

/*
 * The main of the client process
 */
void client_main()
{
    printf("I'm the client, my pid is : %d\n", getpid());
}