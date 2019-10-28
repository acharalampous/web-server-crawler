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
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <signal.h>

#include "myCrawler.h"
#include "aclib.h"
#include "threadPool.h"
#include "uQueue.h"

/*  Implementation of all functions of threadPool
 *  (TP_) Definitions of them found in sQueue.h
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
    newThrPool->numOfDone = 0;
    return newThrPool;
}

void* threadCourse(void* myC){

    int errCode; // collect errors
    myCrawler* crawler = (myCrawler*)myC; // cast server back

    while(crawler->shutdown != 1 || crawler->finished != 1){ // until crawler is shut down or finished crawling

        if((errCode = pthread_mutex_lock(&(crawler->thrPool->pmutex))) != 0){
            printf("Pthread_mutex_lock in thread [%lu]\n", pthread_self());
            exit(1);
        }

        while(crawler->urlQueue->totalNodes < 1){ // while the queue is empty
            if(crawler->shutdown == 1 || crawler->finished == 1){ // server at this point was shutted
                break;
            }

            crawler->thrPool->numOfDone++;

            if(crawler->thrPool->numOfDone == crawler->thrPool->numOfThreads){ // no more pages to download
                crawler->finished = 1;
                break;
            }


            pthread_cond_wait(&(crawler->thrPool->conVar), &(crawler->thrPool->pmutex)); // wait for queue to get element

            if(crawler->shutdown == 1 || crawler->finished == 1){ // server at this point was shutted
                break;
            }
        } // end while queue is empty


        if(crawler->shutdown == 1 || crawler->finished == 1){ // server at this point was shutted
            pthread_cond_signal(&(crawler->thrPool->conVar));
            pthread_mutex_unlock(&(crawler->thrPool->pmutex));
            break;
        }

        crawler->thrPool->numOfDone = 0; // there are still pages to download, reset counter

        char* url = uQ_Pop(crawler->urlQueue); // get fd from queue [CS]
        if((errCode = pthread_mutex_unlock(&(crawler->thrPool->pmutex))) != 0){
            printf("Pthread_mutex_unlock in thread [%lu]\n", pthread_self());
           	exit(1);
        }


        char* host = NULL; // hostName
        int port = -1; // port to connect to host
        char* page = NULL; // page to request

        if(analyzeURL(url, &host, &port, &page) != 0){ // get host,port,page from fullUrl
            free(url);
            if(host != NULL) free(host);
            if(page != NULL) free(page);
            continue;
        }

        int sockfd = socket(AF_INET, SOCK_STREAM, 0); // create socket

        struct sockaddr_in serverAddr;
        getServerAddr(host, port, &serverAddr); // match socket to server address

        char* request = createRequest(host, page); // create GET request
        if(request == NULL){
            free(url);
            free(host);
            free(page);
            close(sockfd);
            continue;
        }

        /* Connect to WebServer */
        if (connect(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0){
            perror("ERROR connecting");
            free(request);
            free(url);
            free(host);
            free(page);
            continue;
        }

        writeAll(sockfd, request); // send request

        char* buffer = readAll(sockfd); // get response

        close(sockfd); // close connection

        /* Next the read http response will be analyzed, returning code and content */
        int code = -1;
        char* content = analyzeHTTP(buffer, &code);

        if(code != 200 || content == NULL){ // response was not OK
            free(request);
            if(content != NULL)
                free(content);
            free(url);
            free(host);
            free(page);
            free(buffer);
            continue;
        }
        else{ // else --> code == 200
            pthread_mutex_lock(&(crawler->thrPool->pmutex));

                int saveDirLen = strlen(crawler->saveDir);
                char* saveDir = myStrCopy(crawler->saveDir, saveDirLen);

            pthread_mutex_unlock(&(crawler->thrPool->pmutex));
            char* temp1 = page; // find the site name
            if(*temp1 == '/')
                temp1++;

            char* temp2 = temp1;
            temp2 = strchr(temp1, '/'); // get end of site
            if(temp2 == NULL){
                free(request);
                free(saveDir);
                free(content);
                free(url);
                free(host);
                free(page);
                free(buffer);
                continue;
            }

            char tempCH = *temp2; // keep character, to switch later back
            *temp2 = '\0';


            int siteNameLen = (temp2 - temp1) + saveDirLen; // get siteName + saveDir length
            char* site = malloc(sizeof(char) * (siteNameLen + 2));
            bzero(site, siteNameLen + 1);

            strcat(site, saveDir);
            strcat(site, "/");
            strcat(site, temp1);
            site[siteNameLen + 1] = '\0'; // create saveDir/siteX

            *temp2 = tempCH; // switch character back

            /* Create folder if it does not exist */
            DIR* dir = opendir(site);
            if(dir == NULL)
                mkdir(site, 0700);
            else
                closedir(dir);

            /* Next the whole path of the page will be created in order to save it to disk */
            int pagePathLen = saveDirLen + strlen(page);
            char* fullPagePath = malloc(sizeof(char) * (pagePathLen + 1));
            bzero(fullPagePath, pagePathLen + 1);

            sprintf(fullPagePath, "%s%s", saveDir, page);
            fullPagePath[pagePathLen] = '\0';

            /* Create new file to paste content downloaded */
            FILE* newFile = fopen(fullPagePath, "wb");
            if(newFile == NULL){
                printf("ERROR - Failed to open file for writing\n");\
                free(buffer);
                free(content);
                free(fullPagePath);
                free(site);
                free(url);
                free(host);
                free(page);
                free(saveDir);
                continue;
            }

            /* Write content to new file */

            fprintf(newFile, "%s", content);

            // Close File
            fclose(newFile);

            printf("Just downloaded: %s - Bytes: %lu\n", fullPagePath, strlen(content));
            if((errCode = pthread_mutex_lock(&(crawler->thrPool->pmutex))) != 0){
                printf("Pthread_mutex_lock in thread [%lu]\n", pthread_self());
                exit(1);
            }
                crawler->bytesDownloaded += strlen(content);
                crawler->pagesDownloaded++;

            if((errCode = pthread_mutex_unlock(&(crawler->thrPool->pmutex))) != 0){
                printf("Pthread_mutex_unlock in thread [%lu]\n", pthread_self());
                exit(1);
            }

            /* Find all links and push them */
            getLinks(crawler, content, host, port, page);

            free(saveDir);
            free(fullPagePath);
            free(site);
        } // end if code == 200

        free(host);
        free(page);
        free(content);
        free(request);
        free(url);
        free(buffer);
    } // end while shutdown/finished

    return NULL;
}


void TP_Destroy(threadPool* thrPool){
    free(thrPool->threads);

    pthread_mutex_destroy(&(thrPool->pmutex));
    pthread_cond_destroy(&(thrPool->conVar));

    free(thrPool);
}
