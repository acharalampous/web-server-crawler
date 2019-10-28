/*******************************/
/* myCrawler.c */

/* Name:    Andreas Charalampous
 * A.M :    1115201500195
 * e-mail:  sdi1500195@di.uoa.gr
 */
/********************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "aclib.h"
#include "myCrawler.h"
#include "uQueue.h"
#include "threadPool.h"

/*  Implementation of all functions of myCrawler
 *  (C_) Definitions of them found in myCrawler.h
 */


myCrawler* C_Init(int numOfThreads, char* saveDir){
    myCrawler* newCrawler = malloc(sizeof(myCrawler));

    if(newCrawler == NULL) // error while allocating space
        return NULL;


    newCrawler->urlQueue = uQ_Init();
    if(newCrawler->urlQueue == NULL)
        return NULL;

    newCrawler->thrPool = TP_Init(numOfThreads);
    if(newCrawler->thrPool == NULL)
        return NULL;

    DIR* dir = opendir(saveDir);
    if(dir == NULL){
        mkdir(saveDir, 0700);
    }
    closedir(dir);
    // Set newServer data //
    newCrawler->saveDir = myStrCopy(saveDir, strlen(saveDir));
    newCrawler->timeStarted = time(NULL);
    newCrawler->bytesDownloaded = 0;
    newCrawler->pagesDownloaded = 0;
    newCrawler->shutdown = 0;
    newCrawler->finished = 0;
    newCrawler->JEStarted = 0;

    return newCrawler;
}


void C_Destroy(myCrawler* crawler){
    free(crawler->saveDir);

    uQ_Destroy(crawler->urlQueue);
    TP_Destroy(crawler->thrPool);

    free(crawler);
}


void C_startThreads(myCrawler* crawler){
    for(int i = 0; i < crawler->thrPool->numOfThreads; i++){
        pthread_create(&(crawler->thrPool->threads[i]), 0, threadCourse, (void*)crawler);
        //pthread_detach(crawler->thrPool->threads[i]);
    }
}

void C_printStats(myCrawler* crawler, int socket){
    int errCode;
    time_t now = time(NULL);

    if((errCode = pthread_mutex_lock(&(crawler->thrPool->pmutex))) != 0){
        printf("Pthread_mutex_lock in main thread: %s", strerror(errCode));
        exit(1);
    }

		int pagesS = crawler->pagesDownloaded;
		int bytesS = crawler->bytesDownloaded;
        time_t elapsed = now - crawler->timeStarted; // compute seconds elapsed

	if((errCode = pthread_mutex_unlock(&(crawler->thrPool->pmutex))) != 0){
        printf("Pthread_mutex_unlock in main thread: %s", strerror(errCode));
        exit(1);
    }

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

   	char toSend[] = "Crawler up for ::.  pages downloaded,  bytes.\n";
	int resLen = strlen(toSend) + 6 + pagesD + bytesD;

	char* response = malloc(sizeof(char) * (resLen + 1));
	bzero(response, resLen + 1);

	sprintf(response, "Crawler up for %s:%s:%s. %d pages downloaded, %d bytes.\n", hrs, min, sec, pagesS, bytesS);

	response[resLen] = '\0';

	writeAll(socket, response);

	free(response);
}
