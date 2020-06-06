#include "client.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/msg.h>
//To use shared memory
#include <sys/ipc.h>
#include <sys/shm.h>
//To use signals
#include <signal.h>
#include <string.h>

/*
 * Client functions.
 */

/*
 * The main of the client process
 */
void client_main()
{
    printf("I'm the Client, my pid is : %d\n", getpid());

    //Get the db shared memory as read-only for queries
    manager_shared_memory = (struct ManagerSharedMemory *)shmat(sys_info.records_shmid, NULL, SHM_RDONLY);
    logger_shared_memory = (struct LoggerSharedMemory *)shmat(sys_info.logger_shmid, NULL, 0);
    //Every client read from his configuration file
     char client_file[6];
     for(int i =1 ; i<=active_clients ;i++)
     {
         if(client_number==i)
         {
            sprintf(client_file, "%d.txt", i);
            read_config_client(client_file);
         }
     }    
    
    //Detach shared memory segments
    shmdt(logger_shared_memory);
    shmdt(manager_shared_memory);
}

/*
 * sends message to the manager, recieves the key back.
 */
void client_add_record(char name[20], int salary, int key)
{
    struct message buff;
    buff.mtype = sys_info.db_manager_pid;
    buff.message_record.key = key;
    buff.message_record.salary = salary;
    buff.type_operation = 1;
    buff.pid = getpid();
    char prod_msg[MAX_LOG_LINE_SIZE];
    sprintf(prod_msg, "Client with pid = %d asked manager to add new record and waiting him to return real key", getpid());
    Produce("Client asked manager to add new record and waiting him to return real key");
    Produce(prod_msg);
    strcpy(buff.message_record.name, name);
    int send_val = msgsnd(sys_info.dbmanager_msgqid, &buff, sizeof(buff)-sizeof(buff.mtype), !IPC_NOWAIT);
    if (send_val == -1)
        perror("Error in send");
    int rec_val = msgrcv(sys_info.dbmanager_msgqid, &buff, sizeof(buff.message_record), getpid(), !IPC_NOWAIT);
    if (rec_val == -1)
        perror("Error in recieve");

    key = buff.message_record.key;
    Produce("Client have received the real key");
    sprintf(prod_msg, "I am Client my pid is = %d and I added record with name %s, salary %d and key %d\n", getpid(), name, salary, key);
    Produce(prod_msg);
}

/*
 * Tell the manager, that there is a record with the certain key, I want to add/sub this value to this salary.
 */
void client_modify(int key, int value)
{
    struct message buff;
    buff.mtype = sys_info.db_manager_pid;
    buff.message_record.key = key;
    buff.type_operation = 2;
    buff.message_record.salary = value;
    buff.pid = getpid();
    Produce("Client asked manager to modify certain record");
    int send_val = msgsnd(sys_info.dbmanager_msgqid, &buff, sizeof(buff)-sizeof(buff.mtype), !IPC_NOWAIT);
    if (send_val == -1)
        perror("Error in send");
}
/*
 * Tell the manager I want to lock a certain record.
 */
void client_acquire(int key)
{
    struct message buff;
    buff.mtype = sys_info.db_manager_pid;
    buff.message_record.key = key;
    buff.type_operation = 3;
    buff.pid = getpid();
    Produce("Client asked manager to acquire lock of certain record");
    int send_val = msgsnd(sys_info.dbmanager_msgqid, &buff, sizeof(buff)-sizeof(buff.mtype), !IPC_NOWAIT);
    if (send_val == -1)
        perror("Error in send");
    int rec_val = msgrcv(sys_info.dbmanager_msgqid, &buff, sizeof(buff)-sizeof(buff.mtype), getpid(), !IPC_NOWAIT);
    if (rec_val == -1)
        perror("Error in recieve");
}

/*
 * Send a key to the manager in a message, release it’s key.
 */
void client_release(int key)
{
    struct message buff;
    buff.mtype = sys_info.db_manager_pid;
    buff.message_record.key = key;
    buff.type_operation = 4;
    buff.pid = getpid();
    Produce("Client asked manager to release lock of certain record");
    int send_val = msgsnd(sys_info.dbmanager_msgqid, &buff, sizeof(buff)-sizeof(buff.mtype), !IPC_NOWAIT);
    if (send_val == -1)
        perror("Error in send");
}

/*
 * Selects all records from the db.
 */
void select_all()
{
    acquire_query_logger_sem();
    //Critical section, write to file the records directly
    //Open file to append in
    FILE *F = fopen(QUERY_LOGGER_FILE_NAME, "a");
    Produce("Opened the query logger file to append to it my queries.\n");
    if (F == NULL)
        perror("Couldn't create file");
    fprintf(F, "I'm process %d and this is the output of my select all query:\n", getpid());
    fprintf(F, "Key\t\t\tName\t\t\tSalary\n");
    for (int i = 0; i < 1000 && manager_shared_memory->records[i].key >= 0; i++)
    {
        fprintf(F, "%d\t\t\t%s\t\t\t%d\n", manager_shared_memory->records[i].key, manager_shared_memory->records[i].name, manager_shared_memory->records[i].salary);
    }
    Produce("Printed the queries!\n");
    //Close the file after writing and release the lock
    fflush(F);
    fclose(F);
    Produce("Closed the query logger file after appending my queries");
    release_query_logger_sem();
}

/*
 * Selects all records from the db starting with name or equal to name depending on exact bool.
 */
void select_name(char *name, int exact)
{
    acquire_query_logger_sem();
    //Critical section, write to file the records directly
    //Open file to append in
    Produce("Opened the query logger file to append to it my queries.\n");
    FILE *F = fopen(QUERY_LOGGER_FILE_NAME, "a");
    if (F == NULL)
        perror("Couldn't create file");
    fprintf(F, "I'm process %d and this is the output of my select name %s, and exact : %d:\n", getpid(), name, exact);
    fprintf(F, "Key\t\t\tName\t\t\tSalary\n");
    for (int i = 0; i < 1000 && manager_shared_memory->records[i].key >= 0; i++)
    {
        if (!check_name(manager_shared_memory->records[i].name, name, exact))
            continue;
        fprintf(F, "%d\t\t\t%s\t\t\t%d\n", manager_shared_memory->records[i].key, manager_shared_memory->records[i].name, manager_shared_memory->records[i].salary);
    }
    Produce("Printed the queries!\n");
    //Close the file after writing and release the lock
    fflush(F);
    fclose(F);
    Produce("Closed the query logger file after appending my queries");
    release_query_logger_sem();
}

/*
 * Selects all records from the db depending on the salary and the mode.
 */
void select_salary(int salary, int mode)
{
    acquire_query_logger_sem();
    //Critical section, write to file the records directly
    //Open file to append in
    Produce("Opened the query logger file to append to it my queries.\n");
    FILE *F = fopen(QUERY_LOGGER_FILE_NAME, "a");
    if (F == NULL)
        perror("Couldn't create file");
    fprintf(F, "I'm process %d and this is the output of my select salary %d, and mode : %d:\n", getpid(), salary, mode);
    fprintf(F, "Key\t\t\tName\t\t\tSalary\n");
    for (int i = 0; i < 1000 && manager_shared_memory->records[i].key >= 0; i++)
    {
        if (!check_salary(manager_shared_memory->records[i].salary, salary, mode))
            continue;

        fprintf(F, "%d\t\t\t%s\t\t\t%d\n", manager_shared_memory->records[i].key, manager_shared_memory->records[i].name, manager_shared_memory->records[i].salary);
    }
    Produce("Printed the queries!\n");
    //Close the file after writing and release the lock
    fflush(F);
    fclose(F);
    Produce("Closed the query logger file after appending my queries");
    release_query_logger_sem();
}

/*
 * Selects all records from the db with hybrid query depending on both salary and name.
 */
void select_hybrid(char *name, int salary, int mode, int exact)
{
    acquire_query_logger_sem();
    //Critical section, write to file the records directly
    //Open file to append in
    Produce("Opened the query logger file to append to it my queries.\n");
    FILE *F = fopen(QUERY_LOGGER_FILE_NAME, "a");
    if (F == NULL)
        perror("Couldn't create file");
    fprintf(F, "I'm process %d and this is the output of my select hybrid, name:  %s, and exact : %d, salary: %d, mode: %d:\n", getpid(), name, exact, salary, mode);
    fprintf(F, "Key\t\t\tName\t\t\tSalary\n");
    for (int i = 0; i < 1000 && manager_shared_memory->records[i].key >= 0; i++)
    {
        if (check_name(manager_shared_memory->records[i].name, name, exact) == 0 || check_salary(manager_shared_memory->records[i].salary, salary, mode) == 0)
            continue;

        fprintf(F, "%d\t\t\t%s\t\t\t%d\n", manager_shared_memory->records[i].key, manager_shared_memory->records[i].name, manager_shared_memory->records[i].salary);
    }
    Produce("Printed the queries!\n");
    //Close the file after writing and release the lock
    fflush(F);
    fclose(F);
    Produce("Closed the query logger file after appending my queries");
    release_query_logger_sem();
}

/*
 * Checks if the condition on the name is true or not.
 */
_Bool check_name(char *real_name, char *name, int exact)
{
    if (exact == 1 && strcmp(name, real_name) != 0)
        return 0;
    if (exact == 0)
    {
        if (strlen(name) > strlen(real_name) || memcmp(name, real_name, strlen(name)) != 0)
            return 0;
    }
    return 1;
}

/*
 * Checks if the condition on the salary is true or not.
 */
_Bool check_salary(int real_salary, int salary, int mode)
{
    if (mode == 0 && !(real_salary == salary))
        return 0;
    if (mode == 1 && !(real_salary > salary))
        return 0;
    if (mode == 2 && !(real_salary < salary))
        return 0;
    if (mode == 3 && !(real_salary >= salary))
        return 0;
    if (mode == 4 && !(real_salary <= salary))
        return 0;
    return 1;
}
/*
read configuration file and make operations in configuration file 
*/
void read_config_client(char file_name[])
{
    FILE *file_pointer;

    file_pointer = fopen(file_name, "r");
    char operation_type[8];
    while (fscanf(file_pointer, "%s", operation_type) != EOF)
    {
        //reset the pointer
        char name[20];
        int salary;
        int key;

        char add[] = "add";
        char modify[] = "modify";
        char acquire[] = "acquire";
        char release[] = "release";
        char query[] = "query";
        char sleep_type[] = "sleep";

        if (strcmp(operation_type, add) == 0)
        {
            fscanf(file_pointer, "%s", name);
            fscanf(file_pointer, "%d", &salary);
            fscanf(file_pointer, "%d", &key);
            client_add_record(name, salary, key);
        }

        if (strcmp(operation_type, modify) == 0)
        {
            fscanf(file_pointer, "%d", &key);
            fscanf(file_pointer, "%d", &salary);
            client_modify(key, salary);
        }

        if (strcmp(operation_type, acquire) == 0)
        {
            fscanf(file_pointer, "%d", &key);
            client_acquire(key);
        }

        if (strcmp(operation_type, release) == 0)
        {
            fscanf(file_pointer, "%d", &key);
            client_release(key);
        }
        if (strcmp(operation_type, sleep_type) == 0)
        {
            fscanf(file_pointer, "%d", &key);
            sleep(key);
        }
        if (strcmp(operation_type, query) == 0)
        {
            char type[8];
            char name_type[] = "name";
            char salary_type[] = "salary";
            char select_type[] = "select";
            char exact_mode[] = "exact";
            char hybrid_mode[] = "hybrid";
            char starts_with_mode[] = "starts_with";
            char exact[12];
            char operator_type[4];
            char operator_1[] = "=";
            char operator_2[] = ">";
            char operator_3[] = "<";
            char operator_4[] = ">=";
            char operator_5[] = "=<";

            fscanf(file_pointer, "%s", type);
            if (strcmp(type, name_type) == 0)
            {
                fscanf(file_pointer, "%s", exact);
                fscanf(file_pointer, "%s", name);
                if (!strcmp(exact, exact_mode))
                    select_name(name, 1);
                else if (!strcmp(exact, starts_with_mode))
                    select_name(name, 0);
            }

            if (strcmp(type, select_type) == 0)
            {
                char all_data[5];
                fscanf(file_pointer, "%s", all_data);
                select_all();
            }
            if (strcmp(type, hybrid_mode) == 0)
            {
                char hybrid_name[8];
                char hybrid_salary[8];
                char hybrid_operator[3];
                fscanf(file_pointer, "%s", hybrid_name);
                fscanf(file_pointer, "%s", exact);
                fscanf(file_pointer, "%s", name);
                fscanf(file_pointer, "%s", hybrid_salary);
                fscanf(file_pointer, "%s", hybrid_operator);
                fscanf(file_pointer, "%d", &salary);
                if (strcmp(hybrid_operator, operator_1) == 0)
                {
                    fscanf(file_pointer, "%d", &salary);

                    if (!strcmp(exact, exact_mode))
                        select_hybrid(name, salary, 0, 1);
                    else if (!strcmp(exact, starts_with_mode))
                        select_hybrid(name, salary, 0, 0);
                }
                if (strcmp(hybrid_operator, operator_2) == 0)
                {
                    fscanf(file_pointer, "%d", &salary);
                    if (!strcmp(exact, exact_mode))
                        select_hybrid(name, salary, 1, 1);
                    else if (!strcmp(exact, starts_with_mode))
                        select_hybrid(name, salary, 1, 0);
                }
                if (strcmp(hybrid_operator, operator_3) == 0)
                {
                    fscanf(file_pointer, "%d", &salary);

                    if (!strcmp(exact, exact_mode))
                        select_hybrid(name, salary, 2, 1);
                    else if (!strcmp(exact, starts_with_mode))
                        select_hybrid(name, salary, 2, 0);
                }
                if (strcmp(hybrid_operator, operator_4) == 0)
                {
                    fscanf(file_pointer, "%d", &salary);
                    if (!strcmp(exact, exact_mode))
                        select_hybrid(name, salary, 3, 1);
                    else if (!strcmp(exact, starts_with_mode))
                        select_hybrid(name, salary, 3, 0);
                }
                if (strcmp(hybrid_operator, operator_5) == 0)
                {
                    fscanf(file_pointer, "%d", &salary);
                    if (!strcmp(exact, exact_mode))
                        select_hybrid(name, salary, 4, 1);
                    else if (!strcmp(exact, starts_with_mode))
                        select_hybrid(name, salary, 4, 0);
                }
            }

            if (strcmp(type, salary_type) == 0)
            {
                fscanf(file_pointer, "%s", operator_type);
                fscanf(file_pointer, "%d", &salary);
                if (strcmp(operator_type, operator_1) == 0)
                {
                    fscanf(file_pointer, "%d", &salary);
                    select_salary(salary, 0);
                }

                if (strcmp(operator_type, operator_3) == 0)
                {
                    fscanf(file_pointer, "%d", &salary);
                    select_salary(salary, 2);
                }
                if (strcmp(operator_type, operator_4) == 0)
                {
                    fscanf(file_pointer, "%d", &salary);
                    select_salary(salary, 3);
                }
                if (strcmp(operator_type, operator_5) == 0)
                {
                    fscanf(file_pointer, "%d", &salary);
                    select_salary(salary, 4);
                }
            }
        }
    }
    fclose(file_pointer);
}
