#ifndef _QUEUE_H_
#define _QUEUE_H_

//Definition of Queue

/*
 * Node structure of linked list.  
 */
struct Node {
    int data;
    struct Node* next; //The next node
};

/*
 * Queue structure consisting of a linked list and a front and rear nodes.  
 */
struct Queue
{
    struct Node *front, *rear; 
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

struct Node* new_node(int d);

struct Queue* create_queue();
void enqueue(struct Queue *q, int d);
int dequeue(struct Queue *q);

#endif /* _QUEUE_H_ */