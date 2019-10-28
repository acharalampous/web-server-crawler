/*******************************/
/* uQueue.c */

/* Name:    Andreas Charalampous
 * A.M :    1115201500195
 * e-mail:  sdi1500195@di.uoa.gr
 */
/********************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "aclib.h"
#include "uQueue.h"

/*  Implementation of all functions of url queue
 *  (uQ_) and its nodes (uQN_). Definitions of them
 *  found in uQueue.h
 */


/////////////////
//** uQ NODE **//
/////////////////
uQNode* uQN_Init(char* newUrl){
    uQNode* newNode = malloc(sizeof(uQNode));

    if(newNode == NULL) // error while allocating space
        return NULL;

    // Set newNode data //
    newNode->url = myStrCopy(newUrl, strlen(newUrl));
    if(newNode->url == NULL){
        return NULL;
    }
    newNode->next = NULL;
    newNode->previous = NULL;

    return newNode;
}


void uQN_Print(uQNode* node){
    printf("url: %s\n", node->url);
}



///////////////////
//** URL QUEUE **//
///////////////////
uQueue* uQ_Init(){
    uQueue* newQueue = malloc(sizeof(uQueue));
    if(newQueue == NULL)
        return NULL; // error during allocation

    newQueue->tableCapacity = 2;
    newQueue->pagesFound = malloc(sizeof(char*) * newQueue->tableCapacity);
    if(newQueue->pagesFound == NULL)
        return NULL;

    for(int i = 0; i < newQueue->tableCapacity; i++)
        newQueue->pagesFound[i] = NULL;

    newQueue->start = NULL;
    newQueue->last = NULL;
    newQueue->totalNodes = 0;
    newQueue->nextCell = 0;

    return newQueue;
}



int uQ_isEmpty(uQueue* q){
    return (q->totalNodes == 0) ? 1 : 0;
}


int uQ_getNumOfNodes(uQueue* q){
    if(q == NULL)
        return -1;
    else
        return q->totalNodes;
}


void uQ_Push(uQueue* q, uQNode* newNode){
    if(uQ_isEmpty(q)){ // empty queue
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


char* uQ_Pop(uQueue* q){
    uQNode* temp = q->last;
    if(temp == NULL) // empty list
        return NULL;

    if(uQ_getNumOfNodes(q) == 1){ // last node in queue is about to be popped
        q->start = NULL;
        q->last = NULL;
    }
    else{
        temp->previous->next = NULL;
        q->last = temp->previous; // new queue last node
    }

    char* toReturn = myStrCopy(temp->url, strlen(temp->url));
    free(temp->url);
    free(temp);

    /* Decrease number of posts */
    q->totalNodes--;

    return toReturn; // success
}


int uQ_checkFound(uQueue* q, char* url){
    int found = 0;
    for(int i = 0; i < (q->nextCell); i++){
        if(!strcmp(q->pagesFound[i], url)){
            found = 1;
            break;
        }
    }
    return found;
}

void uQ_newFound(uQueue* q, char* newUrl){
    if((q->nextCell + 1) > q->tableCapacity){ // table is full
        q->tableCapacity *= 2; // double its capacity
        char** newPagesFound = malloc(sizeof(char*) * q->tableCapacity);
        for(int i = 0; i < q->tableCapacity / 2; i++){
            newPagesFound[i] = q->pagesFound[i]; // copy from first table
        }
        char** toDel = q->pagesFound;
        q->pagesFound = newPagesFound; // point to new table
        free(toDel);
    }

    q->pagesFound[q->nextCell] = myStrCopy(newUrl, strlen(newUrl));
    q->nextCell++; // show next cell to write
}


void uQ_Destroy(uQueue* q){
    uQNode* temp = q->last;
    while(temp != NULL){ // until every node is deleted
        temp = temp->previous; // point to next to delete current
        char* item = uQ_Pop(q); // delete last node
        free(item);
    }

    for(int i = 0; i < q->nextCell; i++)
        free(q->pagesFound[i]);

    free(q->pagesFound);
    free(q);
}


void uQ_Print(uQueue* q){
    uQNode* node = q->start;
    int nodeNum = 1;
    printf("Printing queue: [number of nodes: %d]\n", uQ_getNumOfNodes(q));
    while(node != NULL){
        printf("      +Printing post [%d]: ", nodeNum);
        uQN_Print(node);
        node = node->next;
        nodeNum++;
    }
}
