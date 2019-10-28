/*******************************/
/* acutil.c */

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

#include "aclib.h"


char* analyzeGET(char* request, char* sitesDir){
    if(strncmp(request,"GET ", 4) != 0){ // check for valid request
        return NULL;
    }

    char* site; // site requested
    char* line = myStrCopy(request, strlen(request));
    char* toFree = line; // will be used to free token space
    line = strtok(line, "\n");

    while(line != NULL) {
        if(!strncmp(line,"GET ", 4)){
            line += 4;
            site = strtok(line, " "); // find what site was requested
        }

        line = strtok(NULL, "\n");
    }

    int pathLen = strlen(sitesDir) + strlen(site);
    char* path = malloc(sizeof(char) * pathLen + 1);
    sprintf(path, "%s%s", sitesDir, site);
    path[pathLen] = '\0'; // get full internal path of site requested: dir/site/page

    free(toFree);

    return path;
}

char* httpResponse(char* siteRequested, int* result){
    char response[4096] = {0};
    char* date = getGMTDate();

    char server[] = "Server: myhttpd/1.0.0 (Ubuntu64)";
    char conType[] = "Content-Type: text/html";
    char conn[] = "Connection: Closed";
    char* response1;

    if(access(siteRequested, F_OK) == -1){ // site does not exist
        char protocol[] = "HTTP/1.1 404 Not Found";
        char conLen[26];
        char content[] = "<html>Sorry dude, couldn't find this file.</html>";

        sprintf(conLen, "Content-Length: %lu", strlen(content));
        conLen[strlen(conLen)] = '\0';

        sprintf(response,"%s\n%s\n%s\n%s\n%s\n%s\n\n%s", protocol, date, server, conLen, conType, conn, content);
        response[strlen(response)] = '\0';

        *result = -1;
    }
    else if(access(siteRequested, R_OK) == -1){ // site is not accessible
        char protocol[] = "HTTP/1.1 403 Forbidden";
        char conLen[26];
        char content[] = "<html>Trying to access the file but don't think I can make it.</html>";

        sprintf(conLen, "Content-Length: %lu", strlen(content));
        conLen[strlen(conLen)] = '\0';

        sprintf(response,"%s\n%s\n%s\n%s\n%s\n%s\n\n%s", protocol, date, server, conLen, conType, conn, content);
        response[strlen(response)] = '\0';

        *result = -2;
    }
    else{ // found page and is accessible
        char protocol[] = "HTTP/1.1 200 OK";
        char conLen[] = "Content-Length:";

        FILE* fp = fopen(siteRequested, "r"); // open file
        if(fp == NULL){
            perror("Error opening requested site");
            return NULL;
        }

        char* buffer = NULL;
        size_t len;
        ssize_t bytes_read = getdelim( &buffer, &len, '\0', fp); // load file in memory

        int numOfDigits = 0; // number of digits of the size of file
        int temp = bytes_read;
        while(temp > 0){
            numOfDigits++;
            temp = temp / 10;
        }

        /* Compute and create content length tag */
        char* conLen1;
        conLen1 = malloc(sizeof(char) * strlen(conLen) + 1 + numOfDigits + 1);
        sprintf(conLen1, "%s %lu",conLen, bytes_read);
        conLen1[strlen(conLen) + numOfDigits + 1] = '\0';

        /* Create HTTP response */
        int responseLen = strlen(protocol) + strlen(date) + strlen(server) + strlen(conLen1) + strlen(conType) + strlen(conn) + bytes_read + 7;
        response1 = malloc(sizeof(char) * responseLen + 1);
        sprintf(response1,"%s\n%s\n%s\n%s\n%s\n%s\n\n%s",protocol, date, server, conLen1, conType, conn, buffer);
        response1[responseLen] = '\0';

        *result = (int)bytes_read;

        fclose(fp);
        free(date);
        free(conLen1);
        free(buffer);

        return response1;

    }
    free(date);

    return myStrCopy(response, strlen(response));
}


char* getGMTDate(){
    time_t rawtime;
    struct tm * ti;

    time (&rawtime);
    ti = gmtime (&rawtime); // get current GreenWich time
	char day[3];
	char mon[3];
    char dayN[2];
    char secN[2];
    char minN[2];
    char hourN[2];


	switch(ti->tm_wday){ // Find day
		case(0):
			strcpy(day, "Sun");
			break;

		case(1):
			strcpy(day, "Mon");
			break;
		case(2):
			strcpy(day, "Tue");
			break;
		case(3):
			strcpy(day, "Wed");
			break;
		case(4):
			strcpy(day, "Thu");
			break;
		case(5):
			strcpy(day, "Fri");
			break;
		case(6):
			strcpy(day, "Sat");
			break;
	}

	switch(ti->tm_mon){ // Find month
		case(0):
			strcpy(mon, "Jan");
			break;

		case(1):
			strcpy(mon, "Feb");
			break;
		case(2):
			strcpy(mon, "Mar");
			break;
		case(3):
			strcpy(mon, "Apr");
			break;
		case(4):
			strcpy(mon, "May");
			break;
		case(5):
			strcpy(mon, "Jun");
			break;
		case(6):
			strcpy(mon, "Jul");
			break;
        case(7):
			strcpy(mon, "Aug");
			break;
        case(8):
			strcpy(mon, "Sep");
			break;
        case(9):
			strcpy(mon, "Oct");
			break;
        case(10):
			strcpy(mon, "Nov");
			break;
        case(11):
			strcpy(mon, "Dec");
			break;
	}

    /* Add 0 where necessary */
	if(ti->tm_mday < 10)
        sprintf(dayN, "0%d", ti->tm_mday);
    else
        sprintf(dayN, "%d", ti->tm_mday);

	if(ti->tm_sec < 10)
        sprintf(secN, "0%d", ti->tm_sec);
    else
        sprintf(secN, "%d", ti->tm_sec);

	if(ti->tm_min < 10)
        sprintf(minN, "0%d", ti->tm_min);
    else
        sprintf(minN, "%d", ti->tm_min);

	if(ti->tm_hour < 10)
        sprintf(hourN, "0%d", ti->tm_hour);
    else
        sprintf(hourN, "%d", ti->tm_hour);


    /* Create valid HTTP format of GMT current Time */
	char* fdate = malloc(sizeof(char) * 36 + 1);
	sprintf(fdate, "Date: %s, %s %s %d %s:%s:%s GMT", day, dayN, mon, 1900 + ti->tm_year, hourN, minN, secN);
    fdate[36] = '\0';

	return fdate;
}



void writeAll(int sockfd, char* response){
    int resLen = strlen(response);
    int bytesSent = 0;
    do{ // keep writing until all bytes are sent
        ssize_t written = write(sockfd, &response[bytesSent], resLen + 1 - bytesSent);
        if(written == -1){
            perror("Writing to socket");
            break;
        }
        bytesSent += written;
    } while(bytesSent < resLen + 1);
}


char* readAll(int sockfd){
    int tableCapacity = 2;

    /* Array that holds packs of string, to join them later */
    char** readTable = malloc(sizeof(char*) * tableCapacity);
    if(readTable == NULL){
        perror("readTable malloc");
        return NULL;
    }

    int nextCell = 0;
    int bytesRead = 0;
    int packSize = 500; // may be changed

    int done = 0;
    do{
        /* Check if table has enough capacity for next read, else   */
        /* create a bigger one, copying the content of the old one  */
        if((nextCell + 1) > tableCapacity){ // table is full
            tableCapacity *= 2;
            char** newTable = malloc(sizeof(char*) * tableCapacity);
            if(newTable == NULL){
                perror("newTable malloc");
                return NULL;
            }
            for(int i = 0; i < nextCell; i++)
                newTable[i] = readTable[i]; // copy from first table

            char** toDel = readTable; // keep old table
            readTable = newTable; // point to newTable
            free(toDel); // free old table
        }

        readTable[nextCell] = malloc(sizeof(char) * (packSize + 1));
        if(readTable[nextCell] == NULL){
            perror("read table cell malloc");
            return NULL;
        }

        bzero(readTable[nextCell], packSize + 1);
        int n = read(sockfd, readTable[nextCell], packSize);
        readTable[nextCell][packSize] = '\0';

        if(n == 0){
            nextCell++;
            break;
        }

        if(n <= packSize){
            if(readTable[nextCell][n - 1] == '\0'){ // read last char
                done = 1;
            }
        } // read less bytes

        bytesRead += n;

        nextCell++; // point to next cell to write in


    }while(done != 1);

    char* response = malloc(sizeof(char) * bytesRead);
    bzero(response, bytesRead);

    /* Merge all character read */
    for(int i = 0; i < nextCell; i++){
        if(readTable[i] == NULL)
            break;
        if(i == 0)
            strcpy(response, readTable[i]);
        else
            strcat(response, readTable[i]);
    }

    response[bytesRead - 1] = '\0'; // (avoid edgy situtations)

    /* Free readtable */
    for(int i = 0; i < nextCell; i++)
        if(readTable[i] != NULL)
            free(readTable[i]);

    free(readTable);

    return response;
}


char* myStrCopy(char* src, int srclength){
    char* dest = NULL;
    dest = malloc((srclength + 1) * sizeof(char));
    dest[srclength] = '\0';

    memcpy(dest, src, srclength);
    return dest;
}

int chInStr(char* str, char ch){
    int i = 0;

    while(str[i] != '\0'){
        if(str[i] == ch)
            return 1;
        i++;
    }
    return 0;
}

int isNumber(char* str){
    char* temp = str;
    char ch = *temp;
    if(ch == '-') // negative number is provided
        ch = *(++temp);
    while(ch != '\0'){
        if(ch >= 48 && ch <= 57){
            ch = *(++temp);
        }
        else{
            return 0;
        }
    }
    return 1;
}

int getExeParameters(int argc, char** args, int* servingPort, int* commandPort,
                        int* numOfThreads, char** sitesDir){

    if(argc != 9){
        printf("Error in parameters syntax. More or less parameters given! Abort.\n");
        return -1;
    }
    for (int i = 1; i < argc; i += 2){
        if(args[i][0] != '-'){ // error in parameters
            printf("Error in parameters syntax! Abort.\n");
                return -1;
        }
        char par = args[i][1]; // get parameter
        switch(par){
            case 'p':{ // serving port parameter
                if(*servingPort != -1){ // port given twice
                    printf("Error in parameters! Serving port [-p] is given more than once! Abort.\n");
                    return -2;
                }

                if(!isNumber(args[i + 1])){ // port given is not a number
                    printf("Error in parameters! Serving port given is not a number. Abort\n");
                    return -2;
                }

                *servingPort = atoi(args[i + 1]);
                if(*servingPort <= 0){ // not positive non zero port given
                    printf("Error in parameters! Serving port given is not a positive non-zero number. Abort\n");
                    return -2;
                }
                break;
            }
            case 'c':{ // command port parameter
                if(*commandPort != -1){ // port given twice
                    printf("Error in parameters! Command port [-c] is given more than once! Abort.\n");
                    return -2;
                }

                if(!isNumber(args[i + 1])){ // port given is not a number
                    printf("Error in parameters! Command port given is not a number. Abort\n");
                    return -2;
                }

                *commandPort = atoi(args[i + 1]);
                if(*commandPort <= 0){ // not positive non zero port given
                    printf("Error in parameters! Command port given is not a positive non-zero number. Abort\n");
                    return -2;
                }

                break;
            } // end case -cŕ
            case 't':{ // number of threads parameter
                if(*numOfThreads != -1){ // num of threads given twice
                    printf("Error in parameters! Number of threads [-t] is given more than once! Abort.\n");
                    return -2;
                }

                if(!isNumber(args[i + 1])){ // num of threads is not a number
                    printf("Error in parameters! Number of threads given is not a number. Abort\n");
                    return -2;
                }

                *numOfThreads = atoi(args[i + 1]);
                if(*numOfThreads <= 0){ // not positive non-zero number of threads given
                    printf("Error in parameters! Number of threads is not a positive non-zero number. Abort\n");
                    return -2;
                }

                break;
            } // end case -t
            case 'd':{ // directory of sites parameter
                if(*sitesDir != NULL){ // num of threads given twice
                    printf("Error in parameters! Directory of sites[-d] is given more than once! Abort.\n");
                    return -2;
                }

                *sitesDir = myStrCopy(args[i + 1], strlen(args[i + 1])); // need to be deleted later
                break;
            } // end case -d
            default:{
                printf("Error in parameters. Unknown parameter given [%s]. Abort.\n", args[i]);
                return -5;
            }
        } // end switch
    }

    /* Check that all parameters where correctly provider */
    if(*servingPort == -1 || *commandPort == -1 || *numOfThreads == -1 || *sitesDir == NULL){
        printf("Invalid Parameters! Abort.\n");
        return -1;
    }

    else
        printf("Parameters Given!\nServingPort: %d\nCommandPort: %d\nThreads: %d\nDir: %s\n\n",
                *servingPort, *commandPort, *numOfThreads, *sitesDir);
    return 0; // succesfull check
}

void printValidParameters(){
    printf("\n*Execute again providing the following parameters:\n");
    printf("\t-p servingPort\n");
    printf("\t-c commandPort\n");
    printf("\t-t numOfThreads\n");
    printf("\t-d rootDir\n");
    printf("-servingPort Port Number for server to GET requests, [>=1]\n");
    printf("-commandPort: Port Number for server to get commands, [>=1]\n");
    printf("-numOfThreads: Number of Threads to be created, [>=1]\n");
    printf("-rootDir: Name of the directory that keeps sites\n");
}
