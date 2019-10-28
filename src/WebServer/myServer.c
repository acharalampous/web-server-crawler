/*******************************/
/* myServer.c */

/* Name:    Andreas Charalampous
 * A.M :    1115201500195
 * e-mail:  sdi1500195@di.uoa.gr
 */
/********************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "aclib.h"
#include "myServer.h"
#include "sQueue.h"
#include "threadPool.h"

/*  Implementation of all functions of myServer
 *  (S_) Definitions of them found in myServer.h
 */


myServer* S_Init(int numOfThreads, char* rootDir){
    myServer* newServer = malloc(sizeof(myServer));

    if(newServer == NULL) // error while allocating space
        return NULL;


    newServer->socketQueue = sQ_Init();
    if(newServer->socketQueue == NULL)
        return NULL;

    newServer->thrPool = TP_Init(numOfThreads);


    // Set newServer data //
    newServer->rootDir = myStrCopy(rootDir, strlen(rootDir));
    newServer->timeStarted = time(NULL);
    newServer->bytesServed = 0;
    newServer->pagesServed = 0;
    newServer->shutdown = 0;

    return newServer;
}


void S_Destroy(myServer* server){
    free(server->rootDir);

    sQ_Destroy(server->socketQueue);
    TP_Destroy(server->thrPool);

    free(server);
}


void S_startThreads(myServer* server){
    for(int i = 0; i < server->thrPool->numOfThreads; i++){
        pthread_create(&(server->thrPool->threads[i]), NULL, threadCourse, (void*)server);
    }
}

void S_printStats(myServer* server, int socket){
    int errCode;
    time_t now = time(NULL);
    time_t elapsed = now - server->timeStarted; // compute seconds elapsed

    /* Compute active hours, minutes and seconds */
    int activeHours = elapsed / 3600;
    elapsed = elapsed % 3600;
    int activeMinutes = elapsed / 60;
    int activeSeconds = elapsed % 60; // convert to hh:mm:ss format
    char hrs[2];
    char min[2];
    char sec[2];

    if(activeHours < 10)
        sprintf(hrs,"0%d",activeHours);
    else
        sprintf(hrs,"%d",activeHours);

    if(activeMinutes < 10)
        sprintf(min,"0%d",activeMinutes);
    else
        sprintf(min,"%d",activeMinutes);

    if(activeSeconds < 10)
        sprintf(sec,"0%d",activeSeconds);
    else
        sprintf(sec,"%d",activeSeconds);

    if((errCode = pthread_mutex_lock(&(server->thrPool->pmutex))) != 0){
        printf("Pthread_mutex_lock in main thread: %s", strerror(errCode));
        exit(1);
    }
        /* Get number of pages and bytes server */
		int pagesS = server->pagesServed;
		int bytesS = server->bytesServed;

	if((errCode = pthread_mutex_unlock(&(server->thrPool->pmutex))) != 0){
        printf("Pthread_mutex_unlock in main thread: %s", strerror(errCode));
        exit(1);
    }

	int temp = pagesS;
	int pagesD = 1; // digits of pages number
	while(temp >= 10){
		pagesD++;
		temp = temp / 10;
	}

	temp = bytesS;
	int bytesD = 1; // digits of bytes number
	while(temp >= 10){
		bytesD++;
		temp = temp / 10;
	}


	/* Compute length and create response */
   	char toSend[] = "Server up for ::.  pages served,  bytes.\n";
	int resLen = strlen(toSend) + 6 + pagesD + bytesD;

	char* response = malloc(sizeof(char) * (resLen + 1));
	bzero(response, resLen + 1);

	sprintf(response, "Server up for %s:%s:%s. %d pages served, %d bytes.\n", hrs, min, sec, pagesS, bytesS);

	response[resLen] = '\0';

	/* Finally send response to socket requested STATS*/
    writeAll(socket, response);

	free(response);
}
