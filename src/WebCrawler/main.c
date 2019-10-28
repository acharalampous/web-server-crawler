#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "aclib.h"
#include "uQueue.h"
#include "myCrawler.h"
#include "threadPool.h"
#include "jobExecutor.h"

#define W 1
int main(int argc, char* argv[]){
    char* hostName = NULL;
    int serverPort = -1;
    int commandPort = -1;
    int numOfThreads = -1;
    char* saveDir = NULL;
    char* startingURL = NULL;
    jobExecutor* jobEx = NULL;

    if((getExeParameters(argc, argv, &hostName, &serverPort, &commandPort, &numOfThreads,
                            &saveDir, &startingURL)) != 0){
        printValidParameters();
        return -1;
    }

    myCrawler* crawler = C_Init(numOfThreads, saveDir);
    if(crawler == NULL){
        return -1;
    }

    char* fullURL = createURL(hostName, serverPort, startingURL);

    uQ_Push(crawler->urlQueue, uQN_Init(fullURL));
    uQ_newFound(crawler->urlQueue, fullURL);
    C_startThreads(crawler);

    /* CommandSocket */
    struct sockaddr_in commAddr;
    int commandFD, commandSocket;
    int commAddrLen = sizeof(commAddr);

    /* Creating command socket file descriptor */
    if ((commandFD = socket(AF_INET, SOCK_STREAM, 0)) == 0){
        perror("Command Socket Creation failed");
        exit(EXIT_FAILURE);
    }

    /* Forcefully bind port to socket */
    setsockopt(commandFD, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int));

    commAddr.sin_family = AF_INET;
    commAddr.sin_addr.s_addr = INADDR_ANY;
    commAddr.sin_port = htons(commandPort);

    if (bind(commandFD, (struct sockaddr *)&commAddr, sizeof(commAddr)) < 0){
        perror("Command socket bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(commandFD, 5) < 0){
        perror("Listen to command socket");
        exit(EXIT_FAILURE);
    }

    printf("myCrawler is up and is starting to crawl\n");
    while(1){ // Until Shutdown command is given

        char commandBuffer[4096] = {0};
            if ((commandSocket = accept(commandFD, (struct sockaddr *)&commAddr, (socklen_t*)&commAddrLen))<0){
            perror("Command socket accepting");
            exit(EXIT_FAILURE);
        }

        recv(commandSocket, commandBuffer, sizeof(commandBuffer), 0);

        char* command = myStrCopy(commandBuffer, strlen(commandBuffer));
        char* comm = strtok(command, " \n\r\t"); // Get command

        if(comm == NULL){
            writeAll(commandSocket, "No command given. Available commands: SEARCH / STATS / SHUTDOWN\n");

            close(commandSocket);
			free(command);
            continue;
        }
        if(!strcmp(comm, "SEARCH")){
            pthread_mutex_lock(&(crawler->thrPool->pmutex));

            if(crawler->finished == 0){
                pthread_mutex_unlock(&(crawler->thrPool->pmutex));
                writeAll(commandSocket,"Crawling is still in progress. Try again later.\n");

                close(commandSocket);
				free(command);
                continue;
            }
            else{
                if(crawler->JEStarted == 0){ // Job Executor is not in progress, it will be started right away
                    crawler->JEStarted = 1;
                    pthread_mutex_unlock(&(crawler->thrPool->pmutex));
                    //write(commandSocket,"Your query is being processed. Please wait a moment.\n", 55);
                    jobEx = JE_EXE(W, saveDir);
                }
                else
                    pthread_mutex_unlock(&(crawler->thrPool->pmutex));

                JE_sendCmd(W, jobEx, commandBuffer, commandSocket);

				free(command);
                close(commandSocket);
                continue;
            }
        }
        else if(!strcmp(comm, "STATS")){ // Print stats command
            C_printStats(crawler, commandSocket);
        }
        else if(!strcmp(comm, "SHUTDOWN")){ // Shutdown server command
            writeAll(commandSocket, "Crawler will shutdown in a moment!\n");

			if(crawler->JEStarted == 1){

            	JE_sendCmd(W, jobEx, "EXIT", commandSocket);
            	int wpid, status;
            	while ((wpid = wait(&status)) > 0); // wait for all workers to exit

            	free(jobEx->pipesR);
            	free(jobEx->pipesW);
			}
			free(command);
            close(commandSocket);
            break;
        }
        else
            writeAll(commandSocket, "Invalid command given. Available commands: SEARCH / STATS / SHUTDOWN\n");

		free(command);
        close(commandSocket);
    }

    pthread_mutex_lock(&(crawler->thrPool->pmutex));
        crawler->shutdown = 1;
        pthread_cond_broadcast(&(crawler->thrPool->conVar));
    pthread_mutex_unlock(&(crawler->thrPool->pmutex));

    for(int i = 0; i < numOfThreads; i++){
        pthread_join(crawler->thrPool->threads[i], NULL);
    }

    printf("\nmyCrawler is exiting!\n");

    C_Destroy(crawler);

    free(fullURL);
    free(startingURL);
    free(hostName);
    free(saveDir);
    return 0;

}
