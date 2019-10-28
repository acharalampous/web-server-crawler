/*******************************/
/* uQueue.h */

/* Name:    Andreas Charalampous
 * A.M :    1115201500195
 * e-mail:  sdi1500195@di.uoa.gr
 */
/********************************/
#pragma once


/*  Struct that implements a queue in which URL of pages are written,
 *  in order to be passed to thread, to make request to server for them.
 */

/* Standard model of list-nodes
 * Specs:   * Single-Linked List
 *          * Pointer at start
 *          * FIFO Insertion at start - delete at end
 */


/* Implementation of the URL Queue node */
typedef struct uQNode{
    char* url; // url of page
    struct uQNode* next; // Next node of url queue
    struct uQNode* previous; // previous node of url queue
}uQNode;

/* Implementation of uQueue */
typedef struct uQueue{
    uQNode* start; // points at the first node of queue
    uQNode* last; // points at the last node of queue
    int totalNodes; // number of url waiting to be responded
    char** pagesFound; // table that stores url of pages already found
    int tableCapacity; // number of totals cells in pageFound table
    int nextCell; // next cell to store in pagesFound table
}uQueue;



/***************/
/*** uQ NODE ***/
/***************/
/* All functions about url Queue nodes start with uQN_   */

/* Initializes a new uQNode and returns a pointer to it.    */
/* Takes as parameter the url. Returns NULL if err.         */
uQNode* uQN_Init(char*);

/* Prints info of given uQNode */
void uQN_Print(uQNode*);



/*****************/
/*** URL QUEUE ***/
/*****************/
/* All functions about uQueue start with uQ_ */

/* Initializes a new uQueue and returns a pointer to    */
/* it. Returns NULL in case of error.                   */
uQueue* uQ_Init();

/* Takes as argument a uQueue and returns 1 if empty or 0  */
/* if not.                                                 */
int uQ_isEmpty(uQueue*);

/* Returns the number of sockets of specific queue */
int uQ_getNumOfNodes(uQueue*);

/* Inserts(pushes) node at start of queue */
void uQ_Push(uQueue*, uQNode*);

/* Deletes(pops) the last node. If list is empty, returns -1 */
char* uQ_Pop(uQueue*);

/* Check if given url is already found */
int uQ_checkFound(uQueue*, char*);

/* Place new found page in pagesFound */
void uQ_newFound(uQueue*, char*);

/* Destroy given list and all of its nodes. Removes the last */
/* node, until all nodes are deleted                         */
void uQ_Destroy(uQueue*);

/* Prints info of given queue */
void uQ_Print(uQueue*);
