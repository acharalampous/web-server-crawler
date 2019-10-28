/*******************************/
/* worker.c [Project 3]        */

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
#include <signal.h>

#include "jobExecutor.h"
#include "aclib.h"
#include "txInfo.h"
#include "trie.h"

/* Worker process. Takes as argument the number of process(i) and
 * the two pipeNames for read and write.
 */

int textIndex = 0; // index for mapping text

int worker(int i, char* pNameR, char* pNameW){

    FMap* fileMap = FM_Init();
    Trie* tr = Trie_Init();

    int pWrite = open(pNameW, O_WRONLY);
    int pRead = open(pNameR, O_RDONLY);
    if(pRead < 0 || pWrite < 0){
        printf("[%d] Error opening pipes.Abort\n", getpid());
        close(pRead);
        close(pWrite);
        return -1;
    }

   	int donePaths = 0;
   	while(!donePaths){
      	W_getPath(pRead, pWrite, fileMap, tr, &donePaths);
  	}

/////////////////////////////////////////////////////////////////////////////////

    char command[1024];

    while(1){

        int exitSGN = 0; // exit worker
        int option;

        W_getCommand(pRead, pWrite, command);

        if (!(strncmp(command, "SEARCH", 6))) option = 1;

		else if (!(strncmp(command, "EXIT", 10))) option = 2;

		else{continue;} // read command again


        switch(option){
            case 1:{

                char* keyWords = command + 6; // overpass /search and point to parameters
                discardSpaces(keyWords);
                char* token = myStrCopy(keyWords, strlen(keyWords));
                char* toFree = token;
                token = strtok(token, " \t\n\r");
                while(token != NULL){ // get number of words to check deadline

                    PostingList* ps = Trie_findWord(tr, token); // get posting list of word
                    if(ps == NULL){ //
                        token = strtok(NULL, " \t\n\r"); // get next keyWord
                        continue;
                    }

                    PLNode* currText = ps->start;
                    while(currText != NULL){ // get results for all texted containing word

                        int textIndex = currText->id;
                        linesLN* tempLine = currText->lines;
                        while(tempLine != NULL){ // get all lines of text

                            int lineIndex = tempLine->index;
                            char fullBuff[8192];
                            strcpy(fullBuff, fileMap->fileMap[textIndex]->fullPath);
                            strcat(fullBuff, "/");
                            strcat(fullBuff, fileMap->fileMap[textIndex]->fileName);
                            strcat(fullBuff, " - ");

                            char lineId[5];
                            sprintf(lineId, "%d", lineIndex);
                            strcat(fullBuff, lineId);
                            strcat(fullBuff, ": ");
                            strcat(fullBuff, fileMap->fileMap[textIndex]->lines[lineIndex]); // create full Answer
                            write(pWrite, fullBuff, strlen(fullBuff));

                            char discard[10];
                            read(pRead, discard, 10); // wait acknowledgment

                            tempLine = tempLine->next; // get to next line
                        }
                        currText = currText->next; // get next text
                    }

                    token = strtok(NULL, " \t\n\r");

                }
                char msg[10] = "~~!//";
                write(pWrite, msg, strlen(msg));
                char discard[10];
                read(pRead, discard, 3);

                free(toFree);
                break;
            } // end case 1
            case 2:{
                exitSGN = 1; // exit signal
                break;
            }
        }// end switch
        if(exitSGN == 1)
            break;
	}

    FM_Destroy(fileMap);
    Trie_Destroy(tr);
    close(pRead);
    close(pWrite);

    return 0;
}


void W_getPath(int pRead, int pWrite, FMap* fmap,Trie* tr, int* done){
    char pathName[1024];
    FILE* fd;
    size_t buffsize = 256; // will be used for getline
	char* line = malloc(sizeof(char) * buffsize); // for getline

    int len = read(pRead, pathName, 1024); // read path from JE
    if(len < 0){
        printf("ERR\n");
    }

    pathName[len] = '\0';
    write(pWrite, pathName, strlen(pathName)); // ACK

    if(!strcmp(pathName, "////")){
        *done = 1;
        free(line);
        return;
    }
    /* HERE I GOT A PATHNAME. MUST OPEN ALL OF ITS FILE */

    DIR* dir = opendir(pathName);
    if(dir == NULL){
        printf("ERR dir\n");
    }
    while(1){
        struct dirent* dt = readdir(dir); // get next text file
        if(dt != NULL){
            if(dt->d_ino == 0){ // if not a text file continue
                continue;
            }
        }
        else
            break;

        if((!strcmp(dt->d_name, ".")) || (!strcmp(dt->d_name, "..")))
            continue;
        char* fullName = malloc(sizeof(char) * (strlen(dt->d_name) + strlen(pathName)+ 2));
        strcpy(fullName, pathName);
        strcat(fullName, "/");
        strcat(fullName, dt->d_name); // create full Path/Name

        if((fd = fopen(fullName, "r")) < 0){
            printf("[%d]inv file\n", getpid());
            free(fullName);
            return;
        }
        else{
            /* Get number of lines in file */
            int lines = 0;
            while(!feof(fd)){
                getline(&line, &buffsize, fd); // read line from file
                if(feof(fd))
                    break;

                char* line1 = discardSpaces(line); // clear all tabs and spaces in front of id
                if(!strcmp(line1, "\n")){ // line had only tabs, spaces and \n
                    continue;
                }
                else
                    lines++;
            }

            fseek(fd, 0, SEEK_SET); // found number of lines, point at start

            FM_insertText(fmap, dt->d_name, pathName, lines);

            int numOfLine = 0;
            while(!feof(fd)){
                getline(&line, &buffsize, fd); // read line from file
                TF_incBytes(fmap->fileMap[textIndex], strlen(line));
                if(feof(fd))
                    break;

                char* line1 = discardSpaces(line); // clear all tabs and spaces in front of id
                if(!strcmp(line1, "\n")){ // line had only tabs, spaces and \n
                    continue;
                }
                else
                    FM_insertLine(fmap, line1, numOfLine, textIndex);

                    char* token = myStrCopy(line1, strlen(line1));
                    char* toFree = token; // will be used to free token space
                    token = strtok(token, " \t\n");

                    int totalWords = 0;
                    while(token != NULL) {
                        Trie_Insert(tr, token, textIndex, numOfLine); // insert word in trie
                        totalWords++;
                        token = strtok(NULL, " \t\n");
                    }
                    TF_incWords(fmap->fileMap[textIndex], totalWords);

                    numOfLine++;
                    free(toFree);
            }
            FM_incAll(fmap, textIndex);
            textIndex++;
            free(fullName);
            fclose(fd);
        }
    }
    free(dir);
    free(line);
}

void W_getCommand(int pRead, int pWrite, char* command){
    int len = -1;
    int flag = 0;
    while(1){
        len = read(pRead, command, 1023);
        if(len == -1){
            continue;
        }
        command[len] = '\0';
        if (flag == 1){
            break;
        }

        if(!strcmp(command, "##Get Ready to read command##")){
            char ack[15] = "##READY##";
            write(pWrite, ack, strlen(ack));

            flag = 1; // next reading is the command
        }
    }
}
