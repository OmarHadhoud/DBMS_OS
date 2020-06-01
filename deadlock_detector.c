#include <stdio.h>
#include <unistd.h>

/*
 * The main of the deadlock_detector process
 */
void deadlock_detector_main()
{
    printf("I'm the deadlock_detector, my pid is : %d\n", getpid());
    while(1){}
}