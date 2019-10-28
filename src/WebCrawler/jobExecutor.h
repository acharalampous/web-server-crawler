/*******************************/
/* jobExecutor.h [Project 3]   */

/* Name:    Andreas Charalampous
 * A.M :    1115201500195
 * e-mail:  sdi1500195@di.uoa.gr
 */
/********************************/
#pragma once
#include "trie.h"
#include "txInfo.h"

typedef struct jobExecutor{
    int w; // number of workers
    int* chPid; // array that stores all children's pid
    int* pipesR; // pipes for reading from worker, mapped with chPid
    int* pipesW; // pipes for writing to worker, mapped with chPid
}jobExecutor;


/*  Execute the job executor main (Project 2)   */
jobExecutor* JE_EXE(int, char*);

/*  Job Executor[parent] course */
jobExecutor* jEx(int, int*, char*);

/*  Create a jobExecutor struct and return pointer to it    */
jobExecutor* JE_Init(int, int*, int*, int*);

/*  Inform all workers that they are about to read new command */
void JE_informCmd(int, jobExecutor*);

/*  Send given command to workers, get answers and respond to fd given  */
void JE_sendCmd(int, jobExecutor*, char*, int);

///////////////////////////////////////////////////////////////////

/*  Worker[child] course    */
int worker(int, char*, char*);

/*  Get command from Job Executor   */
void W_getCommand(int, int, char*);

/*  Get path of directory to open and analyze data  */
void W_getPath(int,int,FMap*, Trie*, int*);


