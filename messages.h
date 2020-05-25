#ifndef _MESSAGES_H_
#define _MESSAGES_H_

#include <sys/msg.h>

//Definition of messages structs used in the system.

//Initializing system message
struct system_information; //Forward declaration of system info struct.
struct initializing_msg //The initializing message that contains system data.
{
    long mtype; //The receiver pid
    struct system_information sys_info;
    int role;
};

#endif /* _MESSAGES_H_ */