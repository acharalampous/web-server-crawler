/*******************************/
/* aclib.h */

/* Name:    Andreas Charalampous
 * A.M :    1115201500195
 * e-mail:  sdi1500195@di.uoa.gr
 */
/********************************/
#pragma once
#include <time.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>

#include "myCrawler.h"

/*  Header file for all variant functions and structs used
 *  to complete the webServer application.
 */

/*  Given hostName, Port and page, create a full HTPP url and   */
/*  returns it. If error, returns NULL                          */
char* createURL(char*, int, char*);

/*  Given a full URL, finds the hostName, port and page. If */
/*  success, returns 0 else -1                              */
int analyzeURL(char*, char**, int*, char**);

/*  Given the hostname and site, create a HTTP GET request  */
char* createRequest(char*, char*);

/*  Create a socket and make it ready to connect to given host  */
/*  and port. If err return -1                                  */
void getServerAddr(char*, int, struct sockaddr_in*);

/*  Given a HTTP response, check if tis 200 OK and gets the */
/*  content, else returns NULL                              */
char* analyzeHTTP(char*, int*);

/*  Given a content of page, it searches for all links in it    */
/*  Every link found is pushed in queue to be requested         */
int getLinks(myCrawler*, char*, char*, int, char*);

/*  Check if parameters given during execution are valid,   */
/*  and return them!                                        */
int getExeParameters(int, char**, char**, int*, int*, int*, char**, char**);

/*  Print the valid form of given parameters                */
void printValidParameters();

/*  Write everything from string, to given socket or pipe */
void writeAll(int, char*);

/*  Given a socket, read and return everything written to it */
char* readAll(int);

/*  strcpy Alternative. Allocates spaces and returns pointer*/
/*  to the copied string                                    */
char* myStrCopy(char*, int);

/*  Given a string, it check char-char to see if integer.   */
/*  Is yes, returns 1, else 0.                              */
int isNumber(char* str);

/*  Return number of directories in given path(dir), if */
/*  return -1                                           */
int getNumOfDirs(char*);

char* discardSpaces(char*);
