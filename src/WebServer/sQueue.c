/*******************************/
/* sQueue.c */

/* Name:    Andreas Charalampous
 * A.M :    1115201500195
 * e-mail:  sdi1500195@di.uoa.gr
 */
/********************************/
#include <stdio.h>
#include <stdlib.h>

#include "aclib.h"
#include "sQueue.h"

/*  Implementation of all functions of socket queue
 *  (sQ_) and its nodes (sQN_). Definitions of them
 *  found in sQueue.h
 */


/////////////////
//** sQ NODE **//
/////////////////
sQNode* sQN_Init(int fd){
    sQNode* newNode = malloc(sizeof(sQNode));

    if(newNode == NULL) // error while allocating space
        return NULL;

    // Set newNode data //
    newNode->fd = fd;
    newNode->next = NULL;
    newNode->previous = NULL;

    return newNode;
}


void sQN_Print(sQNode* node){
    printf("fd: %d\n", node->fd);
}



//////////////////////
//** socket QUEUE **//
//////////////////////
sQueue* sQ_Init(){
    sQueue* newQueue = malloc(sizeof(sQueue));
    if(newQueue == NULL)
        return NULL; // error during allocation

    newQueue->start = NULL;
    newQueue->last = NULL;
    newQueue->totalNodes = 0;

    return newQueue;
}


int sQ_isEmpty(sQueue* q){
    return (q->totalNodes == 0) ? 1 : 0;
}

int sQ_getNumOfNodes(sQueue* q){
    if(q == NULL)
        return -1;
    else
        return q->totalNodes;
}


void sQ_Push(sQueue* q, sQNode* newNode){
    if(sQ_isEmpty(q)){ // empty queue
        newNode->next = NULL;
        q->last = newNode;
    }
    else{ // not empty
        newNode->next = q->start; // point to current first
        q->start->previous = newNode; // make previous first point to new first
    }

    newNode->previous = NULL;
    q->start = newNode; // insert at start

    /* Increase number of nodes in queue */
    q->totalNodes++;
}


int sQ_Pop(sQueue* q){
    sQNode* temp = q->last;
    if(temp == NULL) // empty list
        return -1;

    if(sQ_getNumOfNodes(q) == 1){ // last node in queue is about to be popped
        q->start = NULL;
        q->last = NULL;
    }
    else{
        temp->previous->next = NULL;
        q->last = temp->previous; // new queue last node
    }

    int toReturn = temp->fd;
    free(temp);

    /* Decrease number of posts */
    q->totalNodes--;

    return toReturn; // success
}


void sQ_Destroy(sQueue* q){
    sQNode* temp = q->last;
    while(temp != NULL){ // until every node is deleted
        temp = temp->previous; // point to next to delete current
        sQ_Pop(q); // delete last node
    }

    free(q);
}


void sQ_Print(sQueue* q){
    sQNode* node = q->start;
    int nodeNum = 1;
    printf("Printing queue: [number of nodes: %d]\n", sQ_getNumOfNodes(q));
    while(node != NULL){
        printf("      +Printing post [%d]: ", nodeNum);
        sQN_Print(node);
        node = node->next;
        nodeNum++;
    }
}
