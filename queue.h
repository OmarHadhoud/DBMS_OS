#ifndef _QUEUE_H_
#define _QUEUE_H_

//Definition of Queue

/*
 * Node structure of linked list.  
 */
struct node {
    int data;
    struct node* next; //The next node
};

/*
 * Queue structure consisting of a linked list and a front and rear nodes.  
 */
struct queue
{
    struct node *front, *rear; 
};


/*
 * Queue functions.
 * 
 * new_node: Creates a new node. 
 * 
 * create_queue: Creates queue and returns it.
 * enqueue: Adds item d to the end of the queue.
 * dequeue: Gets and removes the first element entered the queue from the existing ones.
 */

struct node* new_node(int d);

struct queue* create_queue();
void enqueue(struct queue *q, int d);
int dequeue(struct queue *q);

#endif /* _QUEUE_H_ */