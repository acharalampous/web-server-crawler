/*******************************/
/* jobExecutor.c [Project 3]   */

/* Name:    Andreas Charalampous
 * A.M :    1115201500195
 * e-mail:  sdi1500195@di.uoa.gr
 */
/********************************/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <time.h>
#include <dirent.h>

#include "aclib.h"
#include "jobExecutor.h"

jobExecutor* JE_EXE(int w, char* saveDir){
    system("rm -f Pipe*"); // remove pipes from before

    int i, pid;
    char pI[5];
    char pNameW[10]; // used to create the name of pipes for W
    char pNameR[10]; // used for pipes for R

    int* chPid; // array that will hold all children pids

    int numOfDirs = getNumOfDirs(saveDir);
    if(numOfDirs == -1){
        printf("ERROR while opening file\n");
        return NULL;
    }
    else if(numOfDirs == 0){
        printf("No paths in given file\n");
        return NULL;
    }

    if(numOfDirs < w) // create only as much workers as paths
        w = numOfDirs;

    chPid = malloc(sizeof(int) * w); // keep all children ids <-> map pid to i

    for(i = 0; i < w; i++){

        sprintf(pI, "%d", i);
        strcpy(pNameW, "Pipe");
        strcpy(pNameR, "Pipe");

        strcat(pNameR, pI);
        strcat(pNameW, pI);

        strcat(pNameR, "[R]");
        strcat(pNameW, "[W]"); // create name for pipes W and R

        mkfifo(pNameR, 0666);
        mkfifo(pNameW, 0666); // create pipes using names

        pid = fork(); // create worker
        if(pid == 0)
            break;
        else
            chPid[i] = pid;
    }

    if(pid == 0){
        free(chPid);
        int totalS = worker(i, pNameW, pNameR); // execute worker process
        exit(totalS);
    }
    else
        return jEx(w, chPid, saveDir); // execute job executor
}

jobExecutor* jEx(int w, int* chPid, char* saveDir){

    char pNameW[15];
    char pNameR[15];

    /*  Read and open all pipes for R and W */
    int* pipesR = malloc(sizeof(int) * w); // used to store all pipes for R
    int* pipesW = malloc(sizeof(int) * w); // used to store all pipes for W
    for(int i = 0; i < w; i++){
        char pI[5];
        sprintf(pI, "%d", i);
        strcpy(pNameW, "Pipe");
        strcpy(pNameR, "Pipe");

        strcat(pNameR, pI);
        strcat(pNameW, pI);

        strcat(pNameR, "[R]");
        strcat(pNameW, "[W]"); // create name of pipes to open

        pipesR[i] = open(pNameR, O_RDONLY);
        pipesW[i] = open(pNameW, O_WRONLY); // open both pipes for R and W
    }

	int i = 0;

    DIR* dir = opendir(saveDir);
    if(dir == NULL){
        return NULL;
    }

	while(1){
        struct dirent* dt = readdir(dir); // get next folder

		if(dt != NULL){ // continue for all files in dir
			if((!strcmp(dt->d_name, ".")) || (!strcmp(dt->d_name, ".."))) // skip . and ..
				continue;

            /* Create full path to current file */
			int len = strlen(saveDir) + strlen(dt->d_name) + 1; // dir/nameOfFile
			char* fullPath = malloc(sizeof(char) * (len + 1));
			bzero(fullPath, len + 1);
			sprintf(fullPath, "%s/%s", saveDir, dt->d_name);
			fullPath[len] = 0;

            /* Check if it is a dir */
			DIR* dr = opendir(fullPath);
			if(dr == NULL){
				free(fullPath);
				continue;
			}
            else{
                write(pipesW[i], fullPath, strlen(fullPath) + 1);
                char ansBuff[1024];

                read(pipesR[i], ansBuff, 1024); // get confirmation from worker
                i = (i + 1) % w; // find next worker to be contacted
            }

            free(fullPath);
            continue;

        }
        else
            break;
	}
    closedir(dir);

    char buff[10] = "////"; // specific string will inform worker that all paths were given

    /* Inform workers that all paths are given */
    for(int i = 0; i < w; i++){
        write(pipesW[i], buff, strlen(buff));
        read(pipesR[i], buff, 1024);
    }

    jobExecutor* je = JE_Init(w, chPid, pipesW, pipesR);
    return je;
}

jobExecutor* JE_Init(int w, int* chPid, int* pipesW, int* pipesR){
    jobExecutor* newJE = malloc(sizeof(jobExecutor));
    if(newJE == NULL)
        return NULL;

    newJE->chPid = chPid;
    newJE->pipesR = pipesR;
    newJE->pipesW = pipesW;
    newJE->w = w;

    return newJE;
}

/*  Used for search command */
void JE_getReady(int w, int* pipesR, int* pipesW, int fd){
    int* pReady = malloc(sizeof(int) * w); // shows which processes finished search
    char buffer[8192] = "";

    /*  Keep track of what pipes are ready to be read   */
    fd_set wrSet;
    fd_set rdSet;

    FD_ZERO(&wrSet);
    FD_ZERO(&rdSet);

    int maxRD = -1;
    int maxWR = -1;

    for(int i = 0; i < w; i++){
        FD_SET(pipesR[i], &rdSet);
        FD_SET(pipesW[i], &wrSet);
        pReady[i] = 0;

        maxRD = (maxRD > pipesR[i]) ? maxRD : pipesR[i];
        maxWR = (maxWR > pipesW[i]) ? maxWR : pipesW[i];
    }

    /*  Get answers from workers */
    int doneW = 0; // number of workers that finished

    while(doneW != w){ // until all workers finised

        select(maxRD + 1, &rdSet, NULL, NULL, NULL);

        for(int k = 0; k < w; k++){

            if(pReady[k] == 1){ // if worker already finished, dont check
                FD_CLR(pipesR[k], &rdSet);
            }

            else if(FD_ISSET(pipesR[k], &rdSet)){ // worker wrote in pipe
                int len;

                len = read(pipesR[k], buffer, 8192);
                buffer[len] = '\0';
                write(pipesW[k], buffer, 3);

                if(!strcmp(buffer, "~~!//")){ // if read this string, current worker finished
                    if(pReady[k] != 1){
                        doneW++;
                        pReady[k] = 1;
                    }

                }
                else{
                    write(fd, buffer, strlen(buffer));
                }

                FD_SET(pipesR[k], &rdSet);
            }
            else
                FD_SET(pipesR[k], &rdSet);
        }
    }
    free(pReady);
}



/*  JE waits from all workers to be ready to recieve a new command  */
/*  If any worker exited, a new one is recreated                    */
void JE_informCmd(int w, jobExecutor* je){
    char buff[30] = "##Get Ready to read command##";
    int len = -1;

    for(int i = 0; i < w; i++){
        int status;
        pid_t result = waitpid(je->chPid[i], &status, WNOHANG);

        if(result == 0){ // child is alive
            while(1){ // until worker is ready

                write(je->pipesW[i], buff, strlen(buff));

                char ack[50];
                len = read(je->pipesR[i], ack, 50);

                if(len == -1){
                    continue;
                }
                ack[len] = '\0';

                if(!strcmp(ack, "##READY##"))
                    break;
            }
        }
    }
}

void JE_sendCmd(int w, jobExecutor* je, char* command, int fd){

    JE_informCmd(w, je);

    int option;

    if (!(strncmp(command, "SEARCH ", 7))) option = 1;

    else if (!(strncmp(command, "EXIT", 4))) option = 2;

    else{return;}


    switch(option){
        case 1:{

            for(int i = 0; i < w; i++){  // send the command to every worker
                write(je->pipesW[i], command, strlen(command));
            }

            JE_getReady(w, je->pipesR, je->pipesW, fd); // get results from workers
            break;
        } // end case 1
        case 2:{
            for(int i = 0; i < w; i++){
                write(je->pipesW[i], "EXIT", strlen("EXIT") + 1);
            }
            break;
        }
    }// end switch:
}
