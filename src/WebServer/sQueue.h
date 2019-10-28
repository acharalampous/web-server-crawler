/*******************************/
/* sQueue.h */

/* Name:    Andreas Charalampous
 * A.M :    1115201500195
 * e-mail:  sdi1500195@di.uoa.gr
 */
/********************************/
#pragma once


/*  Struct that implements a queue in which files descriptors of
 *  sockets are written, in order to be passed to threads to response
 *  to HTTP requests.
 */

/* Standard model of list-nodes
 * Specs:   * Single-Linked List
 *          * Pointer at start
 *          * FIFO Insertion at start - delete at end
 */


/* Implementation of the socket Queue node */
typedef struct sQNode{
    int fd; // file descriptor of socket
    struct sQNode* next; // Next node of socket queue
    struct sQNode* previous; // previous node of socket queue
}sQNode;

/* Implementation of sQueue */
typedef struct sQueue{
    sQNode* start; // points at the first node of queue
    sQNode* last; // points at the last node of queue
    int totalNodes; // number of sockets waiting response
}sQueue;



/***************/
/*** sQ NODE ***/
/***************/
/* All functions about socket Queue nodes start with sQN_   */

/* Initializes a new sQNode and returns a pointer to it.   */
/* Takes as parameter the socket id. Returns NULL if err.  */
sQNode* sQN_Init(int);

/* Prints info of given sQNode */
void sQN_Print(sQNode*);



/********************/
/*** socket QUEUE ***/
/********************/
/* All functions about sQueue start with sQ_ */

/* Initializes a new sQueue and returns a pointer to  */
/* it. Takes as parameter the document id. Returns    */
/* NULL in case of error.                             */
sQueue* sQ_Init();

/* Takes as argument a sQueue and returns 1 if empty or 0  */
/* if not.                                                 */
int sQ_isEmpty(sQueue*);

/* Returns the number of sockets of specific queue */
int sQ_getNumOfNodes(sQueue*);

/* Inserts(pushes) node at start of queue */
void sQ_Push(sQueue*, sQNode*);

/* Deletes(pops) the last node. If list is empty, returns -1 */
int sQ_Pop(sQueue*);

/* Destroy given list and all of its nodes. Removes the last */
/* node, until all nodes are deleted                         */
void sQ_Destroy(sQueue*);

/* Prints info of given queue */
void sQ_Print(sQueue*);

