/*******************************/
/* threadPool.h */

/* Name:    Andreas Charalampous
 * A.M :    1115201500195
 * e-mail:  sdi1500195@di.uoa.gr
 */
/********************************/
#pragma once
#include <pthread.h>

/*  Struct that implements a threadPool. Specifically it starts
 *  and keeps the number of threads given. Those threads will
 *  be used to respond to HTTP requests.
 */


/* Implementation of the socket Queue node */
typedef struct threadPool{
    pthread_t* threads; // all threads
    int numOfThreads; // number of threads kept
    pthread_mutex_t pmutex; // mutex between threads for queue
    pthread_cond_t conVar; // condition variable
}threadPool;


/******************/
/*** THREADPOOL ***/
/******************/
/* All functions about the threadpool start with TP_   */

/* Initializes a new threadPool and returns a pointer to   */
/* it. Takes as parameter the number of threads. Returns   */
/* NULL if err.                                            */
threadPool* TP_Init(int);

/* Destroy given threadPool */
void TP_Destroy(threadPool*);

/* Job of each tread. It is called with pthread_create. */
/* Mulitple threads are executed simultanuously. Each   */
/* waits for a socket fd to be placed in sQueue. Threads*/
/* all working together and access the resources that   */
/* are necessary, using a mutex and condition variable  */
void* threadCourse(void*);

