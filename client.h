#ifndef _CLIENT_H_
#define _CLIENT_H_

#include "manager.h"
#include "parent.h"

//Definition of client process

/*
 * Process variables and structs.  
 */

struct record* client_shm_record;



/*
 * manager process functions.
 * 
 * client_main: initial main function that prints the role.
 * client_add_record: sends message to the manager, recieves the key back.
 * client_modify: Tell the manager, that there is a record with the certain key, I want to add/sub this value to this salary.
 * client_acquire: Tell the manager I want to lock a certain record.
 * client_release: Send a key to the manager in a message, release it’s key.
 */

void client_main();
void client_add_record(char name[20], int salary, int key);
void client_modify(int key, int value);
void client_acquire(int key);
void client_release(int key);


#endif /* _CLIENT_H_ */