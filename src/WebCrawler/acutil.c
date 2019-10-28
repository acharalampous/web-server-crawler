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
#include <dirent.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/param.h>

#include "aclib.h"


char* createURL(char* hostName, int port, char* pageName){
    if(hostName == NULL || port < 0|| pageName == NULL){
        return NULL;
    }
    char prot[] = "http://";
    char* fullURL = NULL;

    int portlen = 1;
    int temp = port;
    while(temp >= 10){
        portlen++;
        temp = temp / 10;
    }

    int urlLen = strlen(prot) + strlen(hostName) + 1 + portlen + strlen(pageName) + 1;
    if(pageName[0] != '/'){
        fullURL = malloc(sizeof(char) * (urlLen + 2)); // length computed before + \0 + / between port and site
        sprintf(fullURL, "%s%s:%d/%s", prot, hostName, port, pageName);
        fullURL[urlLen + 1] = '\0';
    }
    else{
        fullURL = malloc(sizeof(char) * (urlLen + 1)); // length computed before + \0
        sprintf(fullURL, "%s%s:%d%s", prot, hostName, port, pageName);
        fullURL[urlLen] = '\0';
    }

    return fullURL;
}

int analyzeURL(char* fullURL, char** hostName, int* port, char** page){
    if(fullURL == NULL) // null url, return err
        return -1;


    if(strncmp(fullURL, "http://", 7)){ // not a valid http url
        return -1;
    }


    char* hostN = fullURL + 7; // point at start of hostName

    int tempLen = 0; // length of hostName
    char* temp = hostN; // for guiding through string
    char ch = *temp; // current character

    while(ch != '\0' && ch != ':'){ // until eof, or : before port
        tempLen++;
        ch = *(++temp);
    }
    if(ch == '\0' || tempLen == 0){ // invalid url given
        return -1;
    }

    *hostName = malloc(sizeof(char) * (tempLen + 1));
    if(*hostName == NULL){
        return -1;
    }

    bzero(*hostName, tempLen + 1);
    strncpy(*hostName, hostN, tempLen);
    hostName[0][tempLen] = '\0'; // form host Name

    tempLen = 0;
    temp++; // overpass : and point to port num
    char* portN = temp;
    ch = *temp;

    while(ch != '\0' && ch != '/'){ // until eof, or / before site
        tempLen++;
        ch = *(++temp);
    }

    if(ch == '\0' || tempLen == 0){ // invalid url given
        return -1;
    }

    char* portStr = malloc(sizeof(char) * (tempLen + 1));

    bzero(portStr, tempLen + 1);
    strncpy(portStr, portN, tempLen);
    portStr[tempLen] = '\0';

    if(!isNumber(portStr)){ // invalid port number
        return -1;
    }

    *port = atoi(portStr);
    free(portStr);

    int pageLen = strlen(temp);
    *page = malloc(sizeof(char) * (pageLen + 1));
    if(*page == NULL){
        return -1;
    }

    bzero(*page, pageLen + 1);
    strncpy(*page, temp, pageLen);
    page[0][pageLen] = '\0';

    return 0;
}

char* createRequest(char* hostName, char* page){
    int protLen = strlen("GET  HTTP/1.1") + strlen(page);
    char* get = malloc(sizeof(char) * (protLen + 1));

    bzero(get, protLen + 1);
    sprintf(get, "GET %s HTTP/1.1", page);
    get[protLen] = '\0';

    char userAgent[] = "User-Agent: Mozilla/4.0 (compatible; MSIE5; Windows NT)";

    int hostLen = strlen("Host: ") + strlen(hostName);
    char* host = malloc(sizeof(char) * (hostLen + 1));

    bzero(host, hostLen + 1);
    sprintf(host, "Host: %s", hostName);
    host[hostLen] = '\0';

    char lang[] = "Accept-Language: en-us";
    char enc[] = "Accept-Encoding: gzip, deflate";
    char conn[] = "Connection: Keep-Alive";

    int reqLen = protLen + strlen(userAgent) + hostLen + strlen(lang) + strlen(enc) + strlen(conn) + 14;

    char* request = malloc(sizeof(char) * (reqLen + 1));
    bzero(request, reqLen + 1);

    sprintf(request, "%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n\r\n",get,userAgent,host,lang,enc,conn);
    request[reqLen] = '\0';

    free(get);
    free(host);
    return request;

}

void getServerAddr(char* host, int port, struct sockaddr_in* serverAddr){
    char temp[8192];
    int errCode, rc;       /* DNS error code and return code */
    struct hostent* serverDetails;
    struct sockaddr_in sa; // test if valid IP
    struct hostent sD;

    /* Check if valid IP. If yes return 1, else 0 */
    int nameIp = inet_pton(AF_INET, host, &(sa.sin_addr));


    if(nameIp == 0){ // provided host Name
        rc = gethostbyname_r(host, &sD, temp, 8192, &serverDetails, &errCode);
    }
	else{  //provided IP address
        struct in_addr hostAddr;
        inet_aton(host, &hostAddr);
        serverDetails = gethostbyaddr((const char*)&hostAddr, sizeof(hostAddr), AF_INET);
    }

    if(rc != 0)
        return;

    bzero((char *) serverAddr, sizeof(*serverAddr));
    serverAddr->sin_family = AF_INET;
    bcopy((char *)serverDetails->h_addr, (char *)&serverAddr->sin_addr.s_addr, serverDetails->h_length);
    serverAddr->sin_port = htons(port);
}

char* analyzeHTTP(char* buffer, int* code){
    char* temp1 = buffer; // will point at start of each line
    char* temp2 = temp1; // will search for \n
    int lenLen = -1; // number of Digits of the length of content
    char* conLenStr; // will point at the first digit of the content length


    int i = 0;
    int flag = -1; // if it stays -1 it is invalid http
    while(temp1 != NULL){
        temp2 = strchr(temp1, '\n'); // get end of line
        if(temp2 == NULL){ // no new line found
            break;
        }
        else{ // analyze current line
            *temp2 = '\0';
            if(*temp1 == '\0' || (!strcmp(temp1, "\r"))){ // found double \n
                *temp2 = '\n';
                temp1 = temp2 + 1; // point at start of content
                flag = 0;
                break;
            }
            if(i == 0){ // must get http code
                temp1 += 9; // overpass HTTP/1.1
                if(!strncmp(temp1, "200 OK", 6))
                    *code = 200;
                else if(!strncmp(temp1, "404 Not Found", 13))
                    *code = 404;
                else if(!strncmp(temp1, "403 Forbidden", 13))
                    *code = 403;
                i++;
            }
            else{
                if(!strncmp(temp1, "Content-Length: ", 16)){ // get content length
                    temp1 += 16; // point on the first digit of length
                    conLenStr = temp1;
                    lenLen = 0; //
                    char* conLenEnd = temp1; // this will point to the last digit
                    while(*conLenEnd >= 48 && *conLenEnd <= 57){ // while getting integer
                        lenLen++;
                        conLenEnd++;
                    }
                }
            }

            *temp2 = '\n';
            temp1 = temp2 + 1;
        }
    }

    if(flag == -1)
        return NULL;

    /* Character next to content length number will be switched, to isolate length */
    char tmp = conLenStr[lenLen]; // keep character
    conLenStr[lenLen] = '\0';
    int contentLength = atoi(conLenStr);
    conLenStr[lenLen] = tmp; // restore buffer

    /* Copy content */
    char* content = malloc(sizeof(char) * (contentLength + 1));
    sprintf(content, "%s",temp1);
    content[contentLength] = '\0';

    return content;
}

int getLinks(myCrawler* crawler, char* content, char* host, int port, char* page){
    char* temp1 = page; // get site, for creating internal links
    if(*temp1 == '/')
        temp1++;

    char* temp2 = temp1;
    temp2 = strchr(temp1, '/'); // get end of site
    if(temp2 == NULL)
        return -1;

    int siteNameLen = temp2 - temp1;
    char* site = malloc(sizeof(char) * (siteNameLen + 1)); // create siteName
    if(site == NULL)
        return -1;

    bzero(site, siteNameLen + 1);

    strncpy(site, temp1, siteNameLen);
    site[siteNameLen] = '\0';

    temp1 = content;
    temp2 = temp1;

    while(temp1 != NULL){
        temp1 = strchr(temp1, '<'); // get start of HTML tag
        if(temp1 == NULL){ // no new HTML tag found
            break;
        }

        temp2 = strchr(temp1, '>'); // search for end of tag
        if(temp2 == NULL){ // no end found, abort
            break;
        }
        if(!strncmp(temp1, "<a href=", 8)){ // it is a link tag
            temp1 += 8; // point to link

            *temp2 = '\0';

            char* newLink = myStrCopy(temp1, strlen(temp1));

            *temp2 = '>';

            char* newPage = NULL;
            if(!strncmp(newLink, "../", 3)) // external link
                newPage = myStrCopy(newLink + 2, strlen(newLink) - 2); // get page of external link
            else{
                newPage = malloc(sizeof(char*) * (strlen(site) + strlen(newLink) + 2));
                sprintf(newPage, "%s/%s",site, newLink);
                newPage[strlen(site) + strlen(newLink) + 2] = '\0';
            }

            char* newURL = createURL(host, port, newPage); // create full Url
            if((uQ_checkFound(crawler->urlQueue, newURL)) != 1){ // check if link is already found
                pthread_mutex_lock(&(crawler->thrPool->pmutex));
                    uQ_newFound(crawler->urlQueue, newURL); // push to found queue

                    uQ_Push(crawler->urlQueue, uQN_Init(newURL)); // push to request queue
                    pthread_cond_signal(&(crawler->thrPool->conVar)); // signal a thread to start working on new link
                pthread_mutex_unlock(&(crawler->thrPool->pmutex));
            }

            free(newURL);
            free(newPage);
            free(newLink);
        } // end if link

        temp1 = temp2 + 1;
    } // end while

    free(site);

    return 0;
}

int getExeParameters(int argc, char** args, char** hostOrIp, int* serverPort, int* commandPort, int* numOfThreads,
                        char** saveDir, char** startingURL){

    if(argc != 12){
        printf("Error in parameters syntax. More or less parameters given! Abort.\n");
        return -1;
    }
    for (int i = 1; i < argc; i += 2){ // get all parameters
        char par; // parameter given
        if(i == argc - 1){ // last parameter is starting URL;
            par = 's';
        }
        else if(args[i][0] != '-'){ // error in parameters
            printf("Error in parameters syntax! Abort.\n");
            return -1;
        }
        else
            par = args[i][1]; // get parameter
        switch(par){
            case 'h':{ // host or ip given
                if(*hostOrIp != NULL){ // host or ip given twice
                    printf("Error in parameters! Host name[-h] is given more than once! Abort.\n");
                    return -2;
                }

                *hostOrIp = myStrCopy(args[i + 1], strlen(args[i + 1]));
                break;
            } // end case -h
            case 'p':{ // webServer port parameter
                if(*serverPort != -1){ // port given twice
                    printf("Error in parameters! Serving port [-p] is given more than once! Abort.\n");
                    return -2;
                }

                if(!isNumber(args[i + 1])){ // port given is not a number
                    printf("Error in parameters! Serving port given is not a number. Abort\n");
                    return -2;
                }

                *serverPort = atoi(args[i + 1]);
                if(*serverPort <= 0){ // not positive non zero port given
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
            } // end case -c
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
            case 'd':{ // directory of sites to be saved parameter
                if(*saveDir != NULL){ // saveDir given twice
                    printf("Error in parameters! Directory of sites to be saved[-d] is given more than once! Abort.\n");
                    return -2;
                }

                *saveDir = myStrCopy(args[i + 1], strlen(args[i + 1])); // need to be deleted later

                break;
            } // end case -d
            case 's':{
                if(*startingURL != NULL){ // saveDir given twice
                    printf("Error in parameters! Starting URL[last parameter] is given more than once! Abort.\n");
                    return -2;
                }

                *startingURL = myStrCopy(args[i], strlen(args[i])); // need to be deleted later

                break;
            } // end case -s
            default:{
                printf("Error in parameters. Unknown parameter given [%s]. Abort.\n", args[i]);
                return -5;
            }
        } // end switch
    }

    /* Check that all parameters where correctly provided */
    if(*serverPort == -1 || *commandPort == -1 || *numOfThreads == -1 || *saveDir == NULL || *startingURL == NULL || hostOrIp == NULL){
        printf("Invalid Parameters! Abort.\n");
        return -1;
    }

    else
        printf("Parameters Given!\nServingPort: %d\nCommandPort: %d\nThreads: %d\nDir: %s\nStarting URL:%s\n\n",
                *serverPort, *commandPort, *numOfThreads, *saveDir, *startingURL);
    return 0; // succesfull check
}

void printValidParameters(){
    printf("\n*Execute again providing the following parameters:\n");
    printf("\t-h hostOrIp\n");
    printf("\t-p serverPort\n");
    printf("\t-c commandPort\n");
    printf("\t-t numOfThreads\n");
    printf("\t-d saveDir\n");
    printf("\t**startingUrl\n");
    printf("-hostOrIp: Host Name of Machine or Ip of server\n");
    printf("-serverPort: Port Number to server, [>=1]\n");
    printf("-commandPort: Port Number for crawler to get commands, [>=1]\n");
    printf("-numOfThreads: Number of Threads to be created, [>=1]\n");
    printf("-rootDir: Name of the directory that will keep downloaded sites\n");
    printf("-startingUrl, first page to start crawling from, ** must be always given last\n");
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
    int packSize = 500;

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

        nextCell++;


    }while(done != 1);

    char* response = malloc(sizeof(char) * (bytesRead));
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


int getNumOfDirs(char* dirName){
	int numOfFolders = 0;

	DIR* dir = opendir(dirName);
    if(dir == NULL){
        return -1;
    }
    while(1){
        struct dirent* dt = readdir(dir); // get next folder

		if(dt != NULL){ // continue for all files in dir
			if((!strcmp(dt->d_name, ".")) || (!strcmp(dt->d_name, ".."))) // skip . and ..
				continue;

            /* Create full path to current file */
			int len = strlen(dirName) + strlen(dt->d_name) + 1; // dir/nameOfFile
			char* fullPath = malloc(sizeof(char) * (len + 1));
			bzero(fullPath, len + 1);
			sprintf(fullPath, "%s/%s", dirName, dt->d_name);
			fullPath[len] = 0;

            /* Check if it is a dir */
			DIR* dr = opendir(fullPath);
			if(dr == NULL){
				free(fullPath);
				continue;
			}
            else{
                numOfFolders++;
            }

            free(fullPath);
            continue;

        }
        else
            break;
	}
    closedir(dir);
    return numOfFolders;
}

char* discardSpaces(char* src){
    char* delim = src;
    while(*delim != '\n'){
        if(*delim == '\t' || *delim == ' ' )
            delim++;
        else
            break;
    }

    return delim;
}
