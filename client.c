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
    char *msg = "help me please";
    char *msg2 = "help me more";
    logger_shared_memory = (struct LoggerSharedMemory *)shmat(sys_info.logger_shmid, NULL, 0);
    //Produce(msg);
    //Produce(msg2);
    if(client_number==1)
    {
      
      read_config_client("1.txt");
    
    }

    else if(client_number==2)
    {
        read_config_client("2.txt");
     
    
    }
    else if(client_number==3)
    {
      read_config_client("3.txt");
      printf("salary in record 0 = %d\n", manager_shared_memory->records[0].salary) ;
      printf("salary in record 1 = %d\n", manager_shared_memory->records[1].salary) ;
      printf("salary in record 2 = %d\n", manager_shared_memory->records[2].salary) ;
      printf("salary in record 3 = %d\n", manager_shared_memory->records[3].salary) ;
      printf("salary in record 4 = %d\n", manager_shared_memory->records[4].salary) ;
    }
   // else if(client_number==4)
   // {
     //    read_config_client("4.txt");
   // }
    
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
    buff.type_operation = 1;
    buff.pid = getpid();
    
    for (int i = 0; i <= 20; i++)
    {
        if (name[i] >= 'a' && name[i] <= 'z')
            buff.message_record.name[i] = name[i];
    }
    int send_val = msgsnd(sys_info.dbmanager_msgqid, &buff, sizeof(buff), !IPC_NOWAIT);
    if (send_val == -1)
        perror("Error in send");
    printf("I am Client my pid is = %d and I added record \n",getpid());
    int rec_val = msgrcv(sys_info.dbmanager_msgqid, &buff, sizeof(buff.message_record), getpid(), !IPC_NOWAIT);
    if (rec_val == -1)
        perror("Error in recieve");

    key = buff.message_record.key;
     printf("the new key is = %d\n",key);
}

/*
 * Tell the manager, that there is a record with the certain key, I want to add/sub this value to this salary.
 */
void client_modify(int key, int value)
{
    struct message buff;
    buff.mtype = sys_info.db_manager_pid;
    buff.message_record.key = key;
    buff.type_operation =2;
    buff.message_record.salary = value;
    buff.pid = getpid();
    
    int send_val = msgsnd(sys_info.dbmanager_msgqid, &buff, sizeof(buff), !IPC_NOWAIT);
    if (send_val == -1)
        perror("Error in send");

}
/*
 * Tell the manager I want to lock a certain record.
 */
void client_acquire(int key)
{
    printf("in acquire client---------------------------\n");
    struct message buff;
    buff.mtype = sys_info.db_manager_pid;
    buff.message_record.key = key;
    buff.type_operation = 3 ;
    buff.pid = getpid();
    int send_val = msgsnd(sys_info.dbmanager_msgqid, &buff, sizeof(buff), !IPC_NOWAIT);
    if (send_val == -1)
        perror("Error in send");
    printf("hi2\n");    
    int rec_val = msgrcv(sys_info.dbmanager_msgqid, &buff, sizeof(buff), getpid(), !IPC_NOWAIT);
    printf("bye2\n");
    if (rec_val == -1)
        perror("Error in recieve");

}

/*
 * Send a key to the manager in a message, release it’s key.
 */
void client_release(int key)
{
    printf("in release client---------------------------\n");
    struct message buff;
    buff.mtype = sys_info.db_manager_pid;
    buff.message_record.key = key;
    buff.type_operation = 4 ;
    buff.pid = getpid();
    int send_val = msgsnd(sys_info.dbmanager_msgqid, &buff, sizeof(buff),!IPC_NOWAIT);
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
    //Produce("Opened the query logger file to append to it my queries.\n");
    if (F == NULL)
        perror("Couldn't create file");
    struct record *rec = client_shm_records;
    fprintf("I'm process %d and this is the output of my select all query:\n", getpid());
    for (int i = 0; i < 1000; i++)
    {
        fprintf("%d\t%s\t%d\n", rec->key, rec->name, rec->salary);
        fflush(F);
    }
    //Produce("Printed the queries!\n");
    //Close the file after writing and release the lock
    fclose(F);
    //Produce("Closed the query logger file after appending my queries");
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
   // Produce("Opened the query logger file to append to it my queries.\n");
    FILE *F = fopen(QUERY_LOGGER_FILE_NAME, "a");
    if (F == NULL)
        perror("Couldn't create file");
    struct record *rec = client_shm_records;
    fprintf("I'm process %d and this is the output of my select name %s, and exact : %d:\n", getpid(), name, exact);
    for (int i = 0; i < 1000; i++)
    {
        if (!check_name(rec->name,name,exact))
            continue;
        fprintf("%d\t%s\t%d\n", rec->key, rec->name, rec->salary);
        fflush(F);
    }
    //Produce("Printed the queries!\n");
    //Close the file after writing and release the lock
    fclose(F);
    //Produce("Closed the query logger file after appending my queries");
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
  //  Produce("Opened the query logger file to append to it my queries.\n");
    FILE *F = fopen(QUERY_LOGGER_FILE_NAME, "a");
    if (F == NULL)
        perror("Couldn't create file");
    struct record *rec = client_shm_records;
    fprintf("I'm process %d and this is the output of my select salary %s, and mode : %d:\n", getpid(), salary, mode);
    for (int i = 0; i < 1000; i++)
    {
        if (!check_salary(rec->salary,salary,mode))
            continue;

        fprintf("%d\t%s\t%d\n", rec->key, rec->name, rec->salary);
        fflush(F);
    }
   // Produce("Printed the queries!\n");
    //Close the file after writing and release the lock
    fclose(F);
    //Produce("Closed the query logger file after appending my queries");
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
    //Produce("Opened the query logger file to append to it my queries.\n");
    FILE *F = fopen(QUERY_LOGGER_FILE_NAME, "a");
    if (F == NULL)
        perror("Couldn't create file");
    struct record *rec = client_shm_records;
    fprintf("I'm process %d and this is the output of my select hybrid, name:  %s, and exact : %d, salary: %d, mode: %d:\n", getpid(), name, exact, salary, mode);
    for (int i = 0; i < 1000; i++)
    {
        if (check_name(rec->name, name, exact) == 0 || check_salary(rec->salary, salary, mode) == 0)
            continue;

        fprintf("%d\t%s\t%d\n", rec->key, rec->name, rec->salary);
        fflush(F);
    }
    //Produce("Printed the queries!\n");
    //Close the file after writing and release the lock
    fclose(F);
    //Produce("Closed the query logger file after appending my queries");
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
            FILE * file_pointer;
            
      file_pointer = fopen(file_name, "r");
      char operation_type[8] ;
   while(fscanf(file_pointer,"%s", operation_type) != EOF)
{
         //reset the pointer
        char name[20];
        int salary  ;
        int key ;

         char add[]= "add" ;
         char modify[]= "modify" ;
         char acquire []= "acquire" ;
         char release[]= "release" ;
         char query[]= "query" ;
         char sleep_type[] = "sleep";

          if(strcmp(operation_type, add) == 0)
            {    
            	fscanf(file_pointer,"%s",name);
	        fscanf(file_pointer,"%d",&salary);
                 fscanf(file_pointer,"%d",&key);
            client_add_record(name,salary,key);
            }
          
         if(strcmp(operation_type, modify) == 0)
            {
           	fscanf(file_pointer,"%d",&key);
	        fscanf(file_pointer,"%d",&salary);
            client_modify(key,salary);
            }        

         if(strcmp(operation_type, acquire) == 0)
            {
           	fscanf(file_pointer,"%d",&key);
            client_acquire(key) ;
            }
         
          if(strcmp(operation_type, release) == 0)
            {
           	fscanf(file_pointer,"%d",&key);
            client_release(key);
            }
           if(strcmp(operation_type,sleep_type) == 0)
            {
           	fscanf(file_pointer,"%d",&key);
            sleep(key);
            } 
            if(strcmp(operation_type,query) == 0)
	    {
		        char type[8];
		        char name_type[] = "name" ;
		        char salary_type[] = "salary" ;
		        
		   	fscanf(file_pointer,"%s",type);
		        if(strcmp(type,name_type) == 0)
		        {
		        fscanf(file_pointer,"%s",name);
		        printf("%s ", operation_type);
			printf("%s ", type);
		        printf("%s \n", name);
		        }
		       



              if(strcmp(type,salary_type) == 0)
		{
		        char operator_type[4];
		        char operator_1[]="=";
		        char operator_2[]=">";
		        char operator_3[]="<";
		        char operator_4[]=">=";
		        char operator_5[]="=<";      
		        fscanf(file_pointer,"%s",operator_type); 
		        fscanf(file_pointer,"%d",&salary);
		        if(strcmp(operator_type,operator_1) == 0)
		         { 
		            fscanf(file_pointer,"%d",&salary);
		             printf("%s", operation_type);
			     printf("%s", type);
		             printf("%s", operator_type);
		             printf("%d\n", salary);
		          }
		        if(strcmp(operator_type,operator_2) == 0)
		         { 
		            fscanf(file_pointer,"%d",&salary);
		             printf("%s", operation_type);
			     printf("%s", type);
		             printf("%s", operator_type);
		             printf("%d\n", salary);
		          }
 		        if(strcmp(operator_type,operator_3) == 0)
		         { 
		            fscanf(file_pointer,"%d",&salary);
		             printf("%s", operation_type);
			     printf("%s", type);
		             printf("%s", operator_type);
		             printf("%d\n", salary);
		          }
		        if(strcmp(operator_type,operator_4) == 0)
		         { 
		            fscanf(file_pointer,"%d",&salary);
		             printf("%s", operation_type);
			     printf("%s", type);
		             printf("%s", operator_type);
		             printf("%d\n", salary);
		          }
		        if(strcmp(operator_type,operator_5) == 0)
		         { 
		            fscanf(file_pointer,"%d",&salary);
		             printf("%s", operation_type);
			     printf("%s", type);
		             printf("%s", operator_type);
		             printf("%d\n", salary);
		          }
		     }  
            
               }   
   
}
        fclose(file_pointer);

}
