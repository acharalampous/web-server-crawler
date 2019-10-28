/*******************************/
/* threadPool.c */

/* Name:    Andreas Charalampous
 * A.M :    1115201500195
 * e-mail:  sdi1500195@di.uoa.gr
 */
/********************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>

#include "myServer.h"
#include "aclib.h"
#include "threadPool.h"
#include "sQueue.h"

/*  Implementation of all functions of threadPool
 *  (TP_) Definitions of them found in threadPool.h
 */

threadPool* TP_Init(int numOfThreads){
    threadPool* newThrPool = malloc(sizeof(threadPool));
    if(newThrPool == NULL) // error while allocating space
        return NULL;

    newThrPool->threads = malloc(sizeof(pthread_t) * numOfThreads);
    if(newThrPool->threads == NULL) // error while allocating space
        return NULL;

    pthread_mutex_init(&(newThrPool->pmutex), NULL); // init mutex
    pthread_cond_init(&(newThrPool->conVar), NULL); // init condvar

    newThrPool->numOfThreads = numOfThreads;
    return newThrPool;
}

void* threadCourse(void* myS){
    int errCode; // collect errors
    myServer* server = (myServer*)myS; // cast server back

    while(server->shutdown != 1){ // until server is shut down

        if((errCode = pthread_mutex_lock(&(server->thrPool->pmutex))) != 0){
            printf("Pthread_mutex_lock in thread [%lu]: %s", pthread_self(), strerror(errCode));
            exit(1);
        }

        while(server->socketQueue->totalNodes < 1){ // while the queue is empty
            if(server->shutdown == 1){ // server at this point was shutted
                break;
            }

            pthread_cond_wait(&(server->thrPool->conVar), &(server->thrPool->pmutex)); // wait for queue to get element

            if(server->shutdown == 1){ // server was shut down during thread was waiting
                break;
            }
        }

        if(server->shutdown == 1){
            pthread_mutex_unlock(&(server->thrPool->pmutex));
            break;
        }

        int fd = sQ_Pop(server->socketQueue); // get fd from queue [CS]
        if((errCode = pthread_mutex_unlock(&(server->thrPool->pmutex))) != 0){
            printf("Pthread_mutex_unlock in thread [%lu]: %s", pthread_self(), strerror(errCode));
           	exit(1);
        }

        char* servingBuffer = readAll(fd); // read from client

        char* siteRequested = analyzeGET(servingBuffer, server->rootDir); // find what site was requested
        if(siteRequested == NULL){ // invalid site was requested
            free(servingBuffer);
            close(fd);
            continue;
        }

        if(!strcmp(siteRequested, "root/favicon.ico")){ // optional (ignore)
            free(servingBuffer);
            free(siteRequested);
            close(fd);
            continue;
        }

        int result = 0; // if pages was found, result will keep the length of content
        char* response = httpResponse(siteRequested, &result); // get http response to be sent
        if(result > 0){ // if site was found, update server stats
            if((errCode = pthread_mutex_lock(&(server->thrPool->pmutex))) != 0){
                printf("Pthread_mutex_lock in thread [%lu]: %s", pthread_self(), strerror(errCode));
                free(servingBuffer);
                free(siteRequested);
                exit(1);
            }

                printf("Sent: %s - Bytes: %d\n", siteRequested, result);
                server->bytesServed += result;
                server->pagesServed++;

            if((errCode = pthread_mutex_unlock(&(server->thrPool->pmutex))) != 0){
                printf("Pthread_mutex_unlock in thread [%lu]: %s", pthread_self(), strerror(errCode));
                free(servingBuffer);
                free(siteRequested);
                exit(1);
            }
        }


	writeAll(fd, response); // respond to client

	free(servingBuffer);
        free(response);
        free(siteRequested);

        close(fd); // close connection
    }

    return NULL;
}


void TP_Destroy(threadPool* thrPool){
    free(thrPool->threads);

    pthread_mutex_destroy(&(thrPool->pmutex));
    pthread_cond_destroy(&(thrPool->conVar));

    free(thrPool);
}
