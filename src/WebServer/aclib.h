/*******************************/
/* aclib.h */

/* Name:    Andreas Charalampous
 * A.M :    1115201500195
 * e-mail:  sdi1500195@di.uoa.gr
 */
/********************************/
#pragma once
#include <time.h>

#include "sQueue.h"
#include "threadPool.h"

/*  Header file for all variant functions and structs used
 *  to complete the webServer application.
 */


/* Given a HTTP request, find what page was requested, create   */
/* full internal path to it, and return it                      */
char* analyzeGET(char*, char*);

/* Given the requested site name, look if you can access it and */
/* create a HTTP response for the client that requested it.     */
/* Change result according the action, -1: not found, -2: not   */
/* accessible, >0: size of site found in bytes                  */
char* httpResponse(char*, int*);

/* Get current GreenWich time in correct HTTP format    */
char* getGMTDate();

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

/*  Check if parameters given during execution are valid,   */
/*  and return them!                                        */
int getExeParameters(int, char**, int*, int*, int*, char**);

/*  Print the valid form of given parameters                */
void printValidParameters();
