/*******************************/
/* myCrawler.h */

/* Name:    Andreas Charalampous
 * A.M :    1115201500195
 * e-mail:  sdi1500195@di.uoa.gr
 */
/********************************/
#pragma once
#include <time.h>

#include "threadPool.h"
#include "uQueue.h"

/*  Struct that implements a crawler. It keeps a queue to
 *  pass URL of pages to be downloaded, as well as a threadPool
 *  that has threads for downloading those pages. The struct also
 *  has info about the crawler.
 */

typedef struct myCrawler{
    char* saveDir; // directory to store downloaded sites
    time_t timeStarted; // the time the crawler was executed
    int pagesDownloaded; // number of pages served to clients
    int bytesDownloaded; // number of total bytes served
    uQueue* urlQueue; // queue that keeps file descriptors
    threadPool* thrPool; // all threads that will serve clients
    int shutdown; // when 1, server is shutting down
    int finished; // when 1, crawling is done
    int JEStarted; // when 1, job executor is already started
}myCrawler;


/*****************/
/*** myCRAWLER ***/
/*****************/
/* All functions about myCrawler start with C_   */

/* Initializes a crawler given the number of threads    */
/* and the name of directory that all paged will be kept*/
myCrawler* C_Init(int, char*);

/* Destroys given crawler   */
void C_Destroy(myCrawler*);

/* Start the threads that are kept in crawler   */
void C_startThreads(myCrawler*);

/* Print all stats about the given crawler  */
void C_printStats(myCrawler*, int);

