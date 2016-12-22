/*	This is a header file for implementing user defined threads.
 *	All the implementations are provided in "thread.o" library file.
 *	"mythread.o" must be linked during execution.
 */


#ifndef THREAD_H_   /* Inclusion Guard. */
#define THREAD_H_

#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>

#define SECOND 1000000
#define MAX_NO_OF_THREADS 50
#define STACK_SIZE 4096
#define false 0
#define true 1


typedef short bool; 

typedef unsigned int threadid_t;

typedef unsigned long address_t;

typedef void (*sighandler_t) (int);

typedef void* (*funWithArg_t)(void*); 

typedef void (*fun_t)(void);

/* Data structure for recording thread status. */
typedef struct _tstatus{
	
	threadid_t id;
   	enum {RUNNING, READY, SLEEPING, SUSPENDED, TERMINATED} state;
   	unsigned no_of_bursts;
  	unsigned total_exec_time;
   	unsigned total_sleep_time;
   	unsigned avr_exec_time;
   	suseconds_t sleeptime;
   	struct timeval wakeuptime;
   	struct timeval last_exec_time;

}status_t;


/* Data structure for Thread Control Block. */
typedef struct _threadCB{

	threadid_t tid;			/* Thread id */
	status_t stat;			/* Status of the thread */	
	sigjmp_buf env;			/* Context of the thread */
	bool withArg;			/* Check if a thread with arguments was created */
	fun_t fun;			/* Address of the function associated with thread */
	funWithArg_t funWithArg;	/* Address of the function with arguments associated with thread */
	void* arg;			/* Pointer to the arguments passed to the thread */
	void* retval;			/* Return value of the thread */
	struct _threadCB *right_th;	/* Pointer to next/right thread control block */
	struct _threadCB *left_th;	/* Pointer to previous/left thread control block */
	char  *tStack;			/* Stack associated with the thread */

}tcb_t;


/* Data structure of Queue: for listing Thread Control Blocks. */
typedef struct _tqueue{

	int count;			/* Number of thread control blocks in Queue*/
	tcb_t *head;			/* Queue Head */
	tcb_t *tail;			/* Queue Tail */
	
}queue_t;


/* Semaphore structure. */
typedef struct _tsem{

	int value;			/* Value of the semaphore */
	queue_t list;			/* Queue of waiting threads */
	
}semaphore;


/* Creates a new thread. Return Thread ID on success, -1 otherwise. */
int CreateThread (void(*f)(void));


/* Starts the scheduler. */
void Go ();


/* Return the id of the calling thread. */
threadid_t GetMyId ();


/* Deletes a thread. Return 0 on success, -1 otherwise. */
int DeleteThread (threadid_t tid);


/* Signal Handler: Acts as a Scheduler/Dispatcher. */
void Dispatch (int sig);


/* Gives up the CPU for next thread. */
void YieldCPU ();


/* Suspends the thread till its resumed. Return tid on success, -1 otherwise. */
int SuspendThread (threadid_t tid);


/* Resumes a suspended thread till its resumed. Return tid on success, -1 otherwise. */
int ResumeThread (threadid_t tid);


/* Gets the status of the thread with id as tid. Return tid on success, -1 otherwise. */
int GetStatus (int thread_id, status_t *stat);


/* Sends a thread to sleep for sec seconds. */
void SleepThread (int sec);


/* Cleans up the memory usage and shuts down the scheduler. */
void Cleanup ();


/* Creates a new thread with arguments. Return Thread ID on success, -1 otherwise. */
int CreateThreadWithArgs (void *(*f)(void *), void *arg);


/* Get the return value of the thread which returned something. */
void *GetThreadResult (threadid_t tid);


/* Initialization function for semaphore. */
void semInit (semaphore *S, int value);


/* Signal function for semaphore. */
void Signal (semaphore *S);


/* Wait function for semaphore. */
void Wait (semaphore *S);

#endif /* THREAD_H_ ends here. */


	


