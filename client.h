#ifndef _CLIENT_H_
#define _CLIENT_H_

#include "manager.h"
#include "parent.h"
#include "logger.h"
#include "query_logger.h"

//Definition of client process

/*
 * Process variables and structs.  
 */

struct record* client_shm_records;
//The name of the file to write queries in
#define QUERY_LOGGER_FILE_NAME "queries.txt"


/*
 * manager process functions.
 * 
 * client_main: initial main function that prints the role.
 * client_add_record: sends message to the manager, recieves the key back.
 * client_modify: Tell the manager, that there is a record with the certain key, I want to add/sub this value to this salary.
 * client_acquire: Tell the manager I want to lock a certain record.
 * client_release: Send a key to the manager in a message, release it’s key.
 * select_all: Selects all records from the DB.
 * select_name: Select with name starting with or exact name.
 * select_salary: Select with name starting with salary ==,>,<,>=,<=.
 * select_hybrid: Select with name starting with or exact name and with salary condition.
 * check_name: Checks if the condition on the name is true or not.
 * check_salary: Checks if the condition on the salary is true or not.
 */

void client_main();
void client_add_record(char name[20], int salary, int key);
void client_modify(int key, int value);
void client_acquire(int key);
void client_release(int key);
void select_all();
void select_name(char *name, int exact);
void select_salary(int salary, int mode);
void select_hybrid(char *name, int salary, int mode, int exact);

_Bool check_name(char* real_name, char *name, int exact);
_Bool check_salary(int real_salary, int salary, int mode);

#endif /* _CLIENT_H_ */
