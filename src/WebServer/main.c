/************************************/
/* main.c [SYSPRO PROJECT 3/Server] */

/* Name:    Andreas Charalampous
 * A.M :    1115201500195
 * e-mail:  sdi1500195@di.uoa.gr
 */
/************************************/
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>

#include "myServer.h"
#include "aclib.h"


int main(int argc, char* argv[]){
    int servingPort = -1; // port for getting requests
    int commandPort = -1; // port for receiving commands
    int numOfThreads = -1; // number of threads to be kept
    char* sitesDir = NULL; // name of directory keeping websites
    if(getExeParameters(argc, argv, &servingPort, &commandPort, &numOfThreads, &sitesDir) != 0){
        printValidParameters();
        return -1;
    }

    int errCode; // Collect errors
    myServer* server = S_Init(numOfThreads, sitesDir); // Init server
    S_startThreads(server); // and start threads

    /* ServingSocket */
    struct sockaddr_in httpAddr;
    int servingFD, servingSocket;
    int httpAddrLen = sizeof(httpAddr);

    /* CommandSocket */
    struct sockaddr_in commAddr;
    int commandFD, commandSocket;
    int commAddrLen = sizeof(commAddr);


    /* Creating serving socket file descriptor */
    if ((servingFD = socket(AF_INET, SOCK_STREAM, 0)) == 0){
        perror("HTTP Socket Creation failed");
        exit(EXIT_FAILURE);
    }

    /* Creating command socket file descriptor */
    if ((commandFD = socket(AF_INET, SOCK_STREAM, 0)) == 0){
        perror("Command Socket Creation failed");
        exit(EXIT_FAILURE);
    }


    /* Forcefully bind port to socket */
    setsockopt(servingFD, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int));
    setsockopt(commandFD, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int));


    /* Set address */
    httpAddr.sin_family = AF_INET;
    httpAddr.sin_addr.s_addr = INADDR_ANY;
    httpAddr.sin_port = htons(servingPort);

    commAddr.sin_family = AF_INET;
    commAddr.sin_addr.s_addr = INADDR_ANY;
    commAddr.sin_port = htons(commandPort);


    /* Attach socket to port */
    if (bind(servingFD, (struct sockaddr *)&httpAddr, sizeof(httpAddr)) < 0){
        perror("Http socket bind failed");
        exit(EXIT_FAILURE);
    }

    if (bind(commandFD, (struct sockaddr *)&commAddr, sizeof(commAddr)) < 0){
        perror("Command socket bind failed");
        exit(EXIT_FAILURE);
    }


    /* Listen for new connections */
    if (listen(servingFD, 5) < 0){
        perror("Listen to HTTP socket");
        exit(EXIT_FAILURE);
    }

    if (listen(commandFD, 5) < 0){
        perror("Listen to command socket");
        exit(EXIT_FAILURE);
    }

    printf("myServer is up and running!\n");
    while(1){ // Until Shutdown command is given

        /*  Keep track of which socket is ready to be read  */
        fd_set sockets;

        FD_ZERO(&sockets);
        FD_SET(servingFD, &sockets);
        FD_SET(commandFD, &sockets);
        int maxFD = -1;

        /* Get max file descriptor */
        maxFD = (servingFD > commandFD) ? servingFD : commandFD;

        /* Wait for a change in the two sockets */
        select(maxFD + 1, &sockets, NULL, NULL, NULL);
        if(FD_ISSET(servingFD, &sockets)){ // a GET will be received

            if ((servingSocket = accept(servingFD, (struct sockaddr *)&httpAddr, (socklen_t*)&httpAddrLen))<0){
                perror("Serving socket accepting");
                exit(EXIT_FAILURE);
            }

            if((errCode = pthread_mutex_lock(&(server->thrPool->pmutex))) != 0){
                printf("Pthread_mutex_lock in main thread: %s", strerror(errCode));
                exit(1);
            }

                /* Push socket fd to queue [CS] */
                sQ_Push(server->socketQueue, sQN_Init(servingSocket));

                /* Signal thread that queue is not empty */
                pthread_cond_signal(&(server->thrPool->conVar));

            if((errCode = pthread_mutex_unlock(&(server->thrPool->pmutex))) != 0){
                printf("Pthread_mutex_unlock in main thread: %s", strerror(errCode));
                exit(1);
            }

        }
        if(FD_ISSET(commandFD, &sockets)){ // Command will be received

			char commandBuffer[4096] = {0};
            if ((commandSocket = accept(commandFD, (struct sockaddr *)&commAddr, (socklen_t*)&commAddrLen))<0){
                perror("Command socket accepting");
                exit(EXIT_FAILURE);
            }

            recv(commandSocket, commandBuffer, sizeof(commandBuffer), 0);

            char* comm = strtok(commandBuffer, " \n\r\t"); // Get command

            if(comm == NULL){
                writeAll(commandSocket, "No command given. Available commands: SEARCH / STATS / SHUTDOWN\n");

                close(commandSocket);
                continue;
            }
            if(!strcmp(comm, "STATS")){ // Print stats command
                S_printStats(server, commandSocket);
            }
            else if(!strcmp(comm, "SHUTDOWN")){ // Shutdown server command
                writeAll(commandSocket, "Server will shutdown in a moment!\n");

                close(commandSocket);
                break;
            }
            else
                writeAll(commandSocket, "Invalid command given. Available commands: STATS / SHUTDOWN.\n");


            close(commandSocket);
        }
    }

    server->shutdown = 1; // Start shut down sequence
    pthread_cond_broadcast(&(server->thrPool->conVar));
    for(int i = 0; i < server->thrPool->numOfThreads; i++){
        pthread_join(server->thrPool->threads[i], NULL); // Wait for all threads to end their last job
    }

    free(sitesDir);
    S_Destroy(server);
    printf("\nmyServer is exiting!\n");
    return 0;

}
