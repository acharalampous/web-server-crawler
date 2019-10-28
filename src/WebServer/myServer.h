/*******************************/
/* myServer.h */

/* Name:    Andreas Charalampous
 * A.M :    1115201500195
 * e-mail:  sdi1500195@di.uoa.gr
 */
/********************************/
#pragma once
#include <time.h>

#include "sQueue.h"
#include "threadPool.h"

/*  Struct that implements a server. It keeps a queue to
 *  pass fd of sockets to be served, as well as a threadPool
 *  that has threads fo serving HTTP requests. The struct also
 *  has info about the server.
 */

typedef struct myServer{
    char* rootDir; // directory that stores sites
    time_t timeStarted; // the time the server was executed
    int pagesServed; // number of pages served to clients
    int bytesServed; // number of total bytes served
    sQueue* socketQueue; // queue that keeps file descriptors
    threadPool* thrPool; // all threads that will serve clients
    int shutdown; // when 1, server is shutting down
}myServer;


/****************/
/*** mySERVER ***/
/****************/
/* All functions about myServer start with S_   */

/* Initializes a server given the number of threads     */
/* and the name of directory that keeps all the pages   */
myServer* S_Init(int, char*);

/* Destroys given server */
void S_Destroy(myServer*);

/* Start the threads that are kept in server */
void S_startThreads(myServer*);

/* Compute and send stats to socket requested them */
void S_printStats(myServer*, int);



