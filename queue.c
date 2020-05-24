#include "queue.h"

//To allocate and free memory we need stdlib.
#include <stdlib.h>


/*
 * Queue functions.
 */

/*
 *  Creates a node.
 */
struct Node *new_node(int d)
{
    struct Node *tmp = (struct Node*) malloc(sizeof(struct Node)); //Allocates memory for node and assigns its adress to tmp pointer.
    tmp->data = d;
    tmp->next = NULL;
    return tmp;
}

/*
 *  Creates a queue.
 */
struct Queue *create_queue()
{
    struct Queue *tmp = (struct Queue*) malloc(sizeof(struct Queue)); //Allocates memory for queue and assigns its adress to tmp pointer.
    tmp->front = NULL;
    tmp->rear = NULL;
    return tmp;
}

/*
 *  Adds integer d to the rear of the queue.
 */
void enqueue(struct Queue *q, int d)
{
    //Create a node for the new integer.
    struct Node *tmp = new_node(d);

    //Checking if the queue was empty
    if(q->rear == NULL)
    {
        //The front node is the same as the rear which is the new node added.
        q->front = q->rear = tmp;
        return;
    }

    //Update the next node for the rear node, then update the rear node to be the last one inserted.
    q->rear->next = tmp;
    q->rear = tmp;
}

/*
 *  Removes the first entered node from the existing ones and return its data.
 */
int dequeue(struct Queue *q)
{
    //Check if queue was empty
    if(q->front == NULL)
        return -1; //Note we're returning -1 as pid are always a positive number, so its a safe assumption.
    
    //Get a pointer to the node to be removed.
    struct Node *tmp = q->front;
    //Update the front of the queue.
    q->front = tmp->next;

    //If the queue becomes empty, update the rear to be null.
    if(q->front == NULL)
        q->rear = NULL;

    //Save the data in an integer, free the memory of the node then return the integer.
    int d = tmp->data;
    free(tmp);
    return d;
}
