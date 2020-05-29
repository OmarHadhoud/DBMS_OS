#include "client.h"

#include <stdio.h>
#include <string.h>

/*
 * Client functions.
 */

/*
 * The main of the client process
 */
void client_main()
{
    printf("I'm the client, my pid is : %d\n", getpid());
    //Get the db shared memory as read-only for queries
    client_shm_records = (struct ManagerSharedMemory *)shmat(sys_info.records_shmid, NULL, SHM_RDONLY);
    char *msg = "help me please";
    char *msg2 = "help me more";
    logger_shared_memory = (struct LoggerSharedMemory *)shmat(sys_info.logger_shmid, NULL, 0);
    Produce(msg);
    Produce(msg2);
    acquire_query_logger_sem();
    release_query_logger_sem();
    //Detach shared memory segments
    shmdt(client_shm_records);
    shmdt(logger_shared_memory);
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
    buff.pid = getpid();
    for (int i = 0; i <= 20; i++)
    {
        if (name[i] >= 'a' && name[i] <= 'z')
            buff.message_record.name[i] = name[i];
    }
    int send_val = msgsnd(sys_info.dbmanager_msgqid, &buff, sizeof(buff.message_record), 0);
    if (send_val == -1)
        perror("Error in send");

    kill(sys_info.db_manager_pid, SIGCONT);
    raise(SIGSTOP);

    int rec_val = msgrcv(sys_info.dbmanager_msgqid, &buff, sizeof(buff.message_record), getpid(), 0);
    if (rec_val == -1)
        perror("Error in recieve");

    key = buff.message_record.key;
}

/*
 * Tell the manager, that there is a record with the certain key, I want to add/sub this value to this salary.
 */
void client_modify(int key, int value)
{
    struct message buff;
    buff.mtype = sys_info.db_manager_pid;
    buff.message_record.key = key;
    buff.message_record.salary = value;
    buff.pid = getpid();
    int send_val = msgsnd(sys_info.dbmanager_msgqid, &buff, sizeof(buff.message_record), 0);
    if (send_val == -1)
        perror("Error in send");

    kill(sys_info.db_manager_pid, SIGCONT);
    raise(SIGSTOP);
}

/*
 * Tell the manager I want to lock a certain record.
 */
void client_acquire(int key)
{
    struct message buff;
    buff.mtype = sys_info.db_manager_pid;
    buff.message_record.key = key;
    buff.pid = getpid();
    int send_val = msgsnd(sys_info.dbmanager_msgqid, &buff, sizeof(buff.message_record), 0);
    if (send_val == -1)
        perror("Error in send");

    kill(sys_info.db_manager_pid, SIGCONT);
    raise(SIGSTOP);
}

/*
 * Send a key to the manager in a message, release it’s key.
 */
void client_release(int key)
{
    struct message buff;
    buff.mtype = sys_info.db_manager_pid;
    buff.message_record.key = key;
    buff.pid = getpid();
    int send_val = msgsnd(sys_info.dbmanager_msgqid, &buff, sizeof(buff.message_record), 0);
    if (send_val == -1)
        perror("Error in send");

    kill(sys_info.db_manager_pid, SIGCONT);
    raise(SIGSTOP);
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
    if (F == NULL)
        perror("Couldn't create file");
    struct record *rec = client_shm_records;
    for (int i = 0; i < 1000; i++)
    {
        printf("%d\t%c\t%d\n", rec->key, rec->name, rec->salary);
        fflush(F);
    }
    //Close the file after writing and release the lock
    fclose(F);
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
    FILE *F = fopen(QUERY_LOGGER_FILE_NAME, "a");
    if (F == NULL)
        perror("Couldn't create file");
    struct record *rec = client_shm_records;
    for (int i = 0; i < 1000; i++)
    {
        if (!check_name(rec->name,name,exact))
            continue;
        printf("%d\t%c\t%d\n", rec->key, rec->name, rec->salary);
        fflush(F);
    }
    //Close the file after writing and release the lock
    fclose(F);
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
    FILE *F = fopen(QUERY_LOGGER_FILE_NAME, "a");
    if (F == NULL)
        perror("Couldn't create file");
    struct record *rec = client_shm_records;
    for (int i = 0; i < 1000; i++)
    {
        if (!check_salary(rec->salary,salary,mode))
            continue;

        printf("%d\t%c\t%d\n", rec->key, rec->name, rec->salary);
        fflush(F);
    }
    //Close the file after writing and release the lock
    fclose(F);
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
    FILE *F = fopen(QUERY_LOGGER_FILE_NAME, "a");
    if (F == NULL)
        perror("Couldn't create file");
    struct record *rec = client_shm_records;
    for (int i = 0; i < 1000; i++)
    {
        if (check_name(rec->name, name, exact) == 0 || check_salary(rec->salary, salary, mode) == 0)
            continue;

        printf("%d\t%c\t%d\n", rec->key, rec->name, rec->salary);
        fflush(F);
    }
    //Close the file after writing and release the lock
    fclose(F);
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
