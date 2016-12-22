/*	This is an implementation of the functions declared in "thread.h" */

#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>

#include "thread.h"


address_t rotl(address_t ret, int shift)
{
	if((shift  &= 31)==0)
		return ret;
	return (ret << shift) | (ret >> (32 - shift));
}


#ifdef __x86_64__
/* code for 64 bit Intel arch */
typedef unsigned long address_t;
#define JB_SP 6
#define JB_PC 7

address_t encode_addr(address_t addr)
{
	address_t ret;

/*	asm volatile("\tmov	%%fs:0x30,%0" : "=g" (ret));
	ret = ret^addr;
	ret = rotl(ret,17);
return ret;
}

*/
	asm volatile("xorq    %%fs:0x30,%0\n"
        "rolq    $0x11,%0\n"
                 : "=g" (ret)
                 : "0" (addr));

return ret;
}

address_t decode_addr(address_t addr)
{
    	address_t ret;
    	asm volatile("rorq	$0x11,%0\n"

         "xorq	%%fs:0x30,%0\n"
         : "=g" (ret)
         : "0" (addr));

     	return ret;
}

#else
/* code for 32 bit Intel arch */
typedef unsigned long address_t;
#define JB_SP 4
#define JB_PC 5 

address_t encode_addr(address_t addr)
{
    	address_t ret;
/*	asm volatile("\tmov	%%gs:0x18,%0" : "=g" (ret));
	ret = ret^addr;
	ret = rotl(ret,9);
	
return ret;
*/    	asm volatile("xor    %%gs:0x18,%0\n"
        "rol    $0x9,%0\n"
                 : "=g" (ret)
                 : "0" (addr));
return ret; 
}

address_t decode_addr(address_t addr)
{
    	address_t ret;
    	asm volatile("ror $0x09,%0\n"

         "xor %%gs:0x18,%0\n"
         : "=g" (ret)
         : "0" (addr));

     	return ret;
}


#endif


queue_t readyQ, suspendedQ, terminatedQ;

int newCount,pidCount,delCount;

bool qinit,trun,tyield,tsuspend,tsleep;

tcb_t *currentThread;

struct timeval start_time,go_time;

struct timezone tz;

char errormsg = 'E';



void statusInit(status_t *status,threadid_t tid)
{
	suseconds_t sec = 0;
	status->id = tid ;
	status->state = READY;
	status->no_of_bursts = 0;
	status->total_exec_time = 0;
	status->total_sleep_time = 0;
	status->avr_exec_time = 0;
	gettimeofday(&(status->wakeuptime),&tz);
	status->sleeptime = sec;
}


void queueInit(queue_t *Q)
{
	Q->head = NULL;
	Q->tail = NULL;
	Q->count = 0;
}


void Push(tcb_t *T, queue_t *Q)
{	
	if(Q->head == NULL)
		Q->head = Q->tail = T;

	else
	{
		Q->tail->right_th = T;
		T->left_th = Q->tail;
		Q->tail = T;
	}
	Q->count++;	
}

int Pop(tcb_t **T, queue_t *Q)
{
	if(Q->count == 0)
		return -1;
	else
	{
		*T = Q->head;
		Q->head = Q->head->right_th;
		Q->count--;
		
		if(Q->count)
			(Q->head)->left_th = NULL;
		else
			Q->head = Q->tail = NULL;
		(*T)->right_th = NULL;		
		return 0;
	}

}

void wrapper()
{
	if(currentThread->withArg)
	{
		currentThread->retval = (currentThread->funWithArg)(currentThread->arg);
	}
	else
	{
		(currentThread->fun)();
	}

	DeleteThread(currentThread->tid);
}

int CreateThread(fun_t f)
{
	address_t sp, pc;
	suseconds_t alrmtime=ualarm(0,0);
	tcb_t *newThread;
	status_t status;
	
	newThread = (tcb_t *)malloc(sizeof(tcb_t));
	
	if(newThread == NULL)
		return -1;
	
	newThread->tid = pidCount++;
	statusInit(&status,newThread->tid);

	sigsetjmp(newThread->env,1);

	newThread->tStack = (char*)malloc(STACK_SIZE);

		
	newCount++;
	sp = (address_t)newThread->tStack + STACK_SIZE;
 	pc = (address_t)wrapper; 
	newThread->env[0].__jmpbuf[JB_SP] = encode_addr(sp);
	newThread->env[0].__jmpbuf[JB_PC] = encode_addr(pc);

	newThread->stat = status;
	newThread->withArg = false;
	newThread->fun = f;
	newThread->retval = NULL;
	newThread->left_th = NULL;
	newThread->right_th = NULL;

	gettimeofday(&newThread->stat.last_exec_time,&tz);
	
	if(qinit == false)
	{	
		queueInit(&readyQ);
		queueInit(&suspendedQ);
		queueInit(&terminatedQ);
		qinit = true;
	}
	
	Push(newThread,&readyQ);
	if(alrmtime>0)
		ualarm(alrmtime,0);
return newThread->tid ;
}


int CreateThreadWithArgs(funWithArg_t fptr, void *arg)
{
	address_t sp, pc;
	suseconds_t alrmtime=ualarm(0,0);
	tcb_t *newThread;
	status_t status;
	
	newThread = (tcb_t *)malloc(sizeof(tcb_t));
	
	if(newThread == NULL)
		return -1;
	
	newThread->tid = pidCount++;
	statusInit(&status,newThread->tid);

	sigsetjmp(newThread->env,1);

	newThread->tStack = (char*)malloc(STACK_SIZE);

		
	newCount++;
	sp = (address_t)newThread->tStack + STACK_SIZE;
 	pc = (address_t)wrapper; 
	newThread->env[0].__jmpbuf[JB_SP] = encode_addr(sp);
	newThread->env[0].__jmpbuf[JB_PC] = encode_addr(pc);

	newThread->stat = status;
	newThread->withArg = true;
	newThread->arg = arg;
	newThread->funWithArg = fptr; 
	newThread->left_th = NULL;
	newThread->right_th = NULL;

	gettimeofday(&newThread->stat.last_exec_time,&tz);
	
	if(qinit == false)
	{	
		queueInit(&readyQ);
		queueInit(&suspendedQ);
		queueInit(&terminatedQ);
		qinit = true;
	}
	
	Push(newThread,&readyQ);
	if(alrmtime>0)
		ualarm(alrmtime,0);
return newThread->tid ;
}

void *GetThreadResult (threadid_t tid)
{
	tcb_t *t; 
	if (checkById(tid, &terminatedQ)==0)
		for(t=terminatedQ.head ; t ; t=t->right_th)
			if(tid == t->tid)
				return t->retval;
	else
		return &errormsg;
}		


threadid_t GetMyId ()
{
	return currentThread->tid;	
}


int deleteById(threadid_t tid, tcb_t **T, queue_t *Q)
{
	tcb_t *t;
	
	if(Q->count == 0)
		return -1;
	else
	{	
		if(Q->head->tid == tid)
		{
			*T = Q->head;
			Q->count--;
			if(Q->count == 0)
				Q->head = Q->tail = NULL;
			else
			{
				Q->head = Q->head->right_th;
				Q->head->left_th = NULL;
			}	
			return 0;
		}
		else if (Q->tail->tid == tid)
		{
			*T = Q->tail;
			Q->count--;
			if(Q->count == 0)
				Q->head = Q->tail = NULL;
			else
			{
				Q->tail = Q->tail->left_th;
				Q->tail->right_th = NULL;
			}	
			return 0;
		}
		for(t=Q->head ; t!= NULL ; t=t->right_th)
		{
			if(t->tid==tid)
			{
				*T=t;
				t->left_th->right_th = t->right_th;
				t->right_th->left_th = t->left_th;
				Q->count--;
				t->right_th = t->left_th = NULL;
			return 0;
			}
		}
	}
return -1;
}

int DeleteThread(threadid_t tid)
{
	tcb_t *T = NULL ;
	
	if(tid == currentThread->tid)
	{
		trun = false;
		delCount++;
		currentThread->stat.state = TERMINATED;
		printf("DELETED Thread : %d\n",currentThread->tid);
		Push(currentThread,&terminatedQ);
		Dispatch(SIGALRM);
		//ualarm(1,0);
		return 0;
	}
	else if((deleteById(tid,&T,&readyQ) == 0) || (deleteById(tid,&T,&suspendedQ) == 0) )
	{
		T->stat.state = TERMINATED;
		delCount++;
		printf("DELETED Thread : %d\n",T->tid);
		Push(T,&terminatedQ);
		return 0;
	}
	else if (deleteById(tid,&T,&terminatedQ) == 0)
	{
		Push(T,&terminatedQ);
		return 0;
	}	
return -1;
}


void YieldCPU ()
{
	ualarm(0,0);
	tyield = true;
	trun = false;
	Dispatch(SIGALRM);
}


int checkById(threadid_t tid, queue_t *Q)
{
	tcb_t *t;
	for(t=Q->head; t ; t=t->right_th)
		if(t->tid == tid)
			return 0;
	return -1;
}


int SuspendThread (threadid_t tid)
{
	tcb_t *T = NULL ;
	struct timeval currtime;
	
	if(tid == currentThread->tid)
	{
		tsuspend = true;
		currentThread->stat.state = SUSPENDED;
		printf("SUSPENDED Thread : %d\n",currentThread->tid);
		ualarm(0,0);
		YieldCPU();
		return tid;
	}
	
	else if(deleteById(tid,&T,&readyQ) == 0)
	{
		T->stat.state=SUSPENDED;
		printf("SUSPENDED Thread : %d\n",T->tid);
		Push(T,&suspendedQ);
		return tid;
	}

	else if(checkById(tid,&suspendedQ) == 0)
		return tid;

return -1;
}


int ResumeThread(threadid_t tid)
{
	tcb_t *R = NULL;
	struct timeval currtime;
	
	if(checkById(tid,&readyQ) == 0) 
		return tid;
	
	else
	{
		if(deleteById(tid,&R,&suspendedQ) == -1)
			return -1;
				
		else 
		{
			R->stat.state=READY;
			printf("RESUMED Thread : %d\n",R->tid);
			Push(R,&readyQ);
			return tid;
		}
	}
}


void SleepThread (int sec)
{
	ualarm(0,0);
	struct timeval tv;
	tsleep = true;
	
	currentThread->stat.sleeptime = sec;
	gettimeofday(&tv,&tz);
//	currentThread->stat.wakeuptime.tv_usec = tv.tv_usec;
	currentThread->stat.wakeuptime.tv_sec = tv.tv_sec + sec;
//	printf("\nwake uptime @ %d",(int)currentThread->stat.wakeuptime.tv_sec);
	currentThread->stat.total_sleep_time += (currentThread->stat.wakeuptime.tv_sec - tv.tv_sec)*1000;	
	Dispatch(SIGALRM);
}	

void print_stat(status_t stat)
{
	int i=0;
	printf("\n\nUsage stat for Thread : %d",stat.id);
	printf("\n");
	for(;i<25;printf("="),++i);
	printf("\n");
	printf("Current state : ");
	switch(stat.state)
	{
		case RUNNING :printf("RUNNING\n");break;
		case SLEEPING :printf("SLEEPING\n");break;
		case SUSPENDED :printf("SUSPENDED\n");break;
		case READY :printf("READY\n");break;
		default: printf("TERMINATED\n");
	}

	printf("Number of bursts : %d\n",stat.no_of_bursts);
	printf("Total execution time : %d msec\n",stat.total_exec_time);
	printf("Total sleeping time : %d msec\n",stat.total_sleep_time);
	printf("Average execution time : %d msec\n\n",stat.avr_exec_time);
}

	
void print_Q(queue_t *Q)
{
	tcb_t *t;
	if(Q->count==0)
		printf("\nEmpty\n");
	else
	{
		for (t=Q->head; t; t=t->right_th)
			printf("\t%d",t->tid);
		printf("\nCount=%d\n",Q->count);
	}
}

int GetStatus (int tid, status_t *stat)
{
	tcb_t *t;
	
	if(checkById(tid, &readyQ)==0)
	{
		for(t=readyQ.head; t ; t=t->right_th)
		{
			if(t->tid == tid)
			{
				*(stat) = t->stat;
				return tid;
			}	
		}
	}
	else if(checkById(tid, &suspendedQ)==0)
	{
		for(t=suspendedQ.head; t ; t=t->right_th)
		{
			if(t->tid == tid)
			{
				*(stat) = t->stat;
				return tid;
			}	
		}
	}
	else if(checkById(tid, &terminatedQ)==0)
	{
		for(t=terminatedQ.head; t ; t=t->right_th)
		{
			if(t->tid == tid)
			{
				*(stat) = t->stat;
				return tid;
			}	
		}
	}
	else
		return -1;

}



void CleanUp()
{
	struct timeval end_time;
	tcb_t *delThread;
	status_t stat;
	
	ualarm(0,0);
	
	gettimeofday(&end_time,&tz);
	printf("\n%d threads were created",newCount);
	printf("\n%d threads were deleted",delCount);
	
	printf("\nTime elapsed since Go() : ");
	printf("%d Sec",(int)(end_time.tv_sec - go_time.tv_sec));
	
//	if(trun == true)
	{
		print_stat(currentThread->stat);
		usleep(SECOND);
		free(currentThread);
	}
	while (Pop(&delThread,&readyQ) != -1)
	{
		if(GetStatus(delThread->tid, &stat))
			print_stat(delThread->stat);
		usleep(SECOND);
		free(delThread);
	}
	while (Pop(&delThread,&suspendedQ) != -1)
	{
		if(GetStatus(delThread->tid, &stat))
			print_stat(delThread->stat);
		usleep(SECOND);
		free(delThread);
	}
	
	while (Pop(&delThread,&terminatedQ) != -1)
	{
		if(GetStatus(delThread->tid, &stat))
			print_stat(delThread->stat);
		usleep(SECOND);
		free(delThread);
	}

		
	printf("\n\nAll threads cleaned. System stopped ... \n\n");
	
	exit(0);
}



int deleteByTime(struct timeval time,tcb_t **T,queue_t *Q)
{


	if(Q->count == 0)
		return -1;
	
	tcb_t *temp;
	bool check = false;	
	suseconds_t utime = time.tv_sec;
	
	for(temp = Q->head; temp; temp=temp->right_th)
	{
		if(temp->stat.wakeuptime.tv_sec <= utime)
		{
			temp->stat.wakeuptime.tv_usec=0;
			temp->stat.wakeuptime.tv_sec=0;
			temp->stat.sleeptime=0;
			Pop(T,Q);
			Push(temp,&readyQ);
			check = true;
		}
	}
	if(check)
		return 0;
	else
		return -1;
		
		
}



void Dispatch(int signum)
{
	
	struct timeval time;
	gettimeofday(&time,&tz);
	int k;
	
	currentThread->stat.total_exec_time += (time.tv_sec - start_time.tv_sec)*1000 + (time.tv_usec - start_time.tv_usec)/1000;
	currentThread->stat.avr_exec_time = currentThread->stat.total_exec_time/currentThread->stat.no_of_bursts;
	currentThread->stat.last_exec_time = start_time;
	
	if(sigsetjmp(currentThread->env,1) == 1)
		return;
		
	if(readyQ.count == 0)
	{
			//printf("\nNo thread to run..\n");
			if (trun == false)
				CleanUp();
			alarm(2);
			//usleep(SECOND);
	}
	else
	{
		if(tyield == true)
		{
			if(tsuspend == true)
			{
				tsuspend = false;
				tyield = false;
				currentThread->stat.state = SUSPENDED;
				Push(currentThread, &suspendedQ);
			}
			else
			{
				tyield = false;
				currentThread->stat.state = READY;
				Push(currentThread, &readyQ);
			}
		}
		else if(tsleep == true)
		{
			tsleep = false;
			currentThread->stat.state = SLEEPING;
			Push(currentThread, &readyQ);
		}
		else if(trun == true)
		{
			currentThread->stat.state=READY;
			Push(currentThread, &readyQ);
		}
		
next:		Pop(&currentThread,&readyQ);
		if(currentThread->stat.state == SLEEPING)
		{
			if (time.tv_sec >= currentThread->stat.wakeuptime.tv_sec)
			{		
				currentThread->stat.state=RUNNING;	
				trun = true;
				alarm(2);
				gettimeofday(&start_time,&tz);
				//printfcurrentThread->stat.wakeuptime.tv_sec
				
				currentThread->stat.no_of_bursts++;
				printf("\n");
				for(k=0;k<20;k++,printf("-"));
  				printf("\n");
				siglongjmp(currentThread->env,1);
				
			}
			else
			{
				Push(currentThread, &readyQ);
				goto next;
			}
		}
		else
		{
			currentThread->stat.state=RUNNING;	
			trun = true;
			alarm(2);
			gettimeofday(&start_time,&tz);
			currentThread->stat.no_of_bursts++;
			printf("\n");
			for(k=0;k<20;k++,printf("-"));
  			printf("\n");
			siglongjmp(currentThread->env,1);
		}
	}
	
}





void _handler(int sig)
{
	ualarm(0,0);
	CleanUp();
	//exit(0);
}


void Go ()
{
	int i=0;
	gettimeofday(&go_time,&tz);
	
	signal(SIGALRM, Dispatch);
	signal(SIGINT, _handler);
	
	while(readyQ.count == 0);
	
	printf("\n\nExecution Begins\n");
	for(;i<25;printf("="),++i);
	printf("\n");
	
	Pop(&currentThread, &readyQ);
	trun=true;
	alarm(2);
	
	currentThread->stat.no_of_bursts++;
	
	gettimeofday(&currentThread->stat.last_exec_time,&tz);
	gettimeofday(&start_time,&tz);

	siglongjmp(currentThread->env, 1);
}


void semInit(semaphore *S, int val)
{
	S->value = val;
	queueInit(&(S->list));
}

void Wait(semaphore *S)
{
/*	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGALRM);
*/
	S->value--;
	if( S->value < 0 )
	{
		tsuspend = true;
		trun = false;
		currentThread->stat.state = SUSPENDED;
		Push(currentThread ,&(S->list)); 
		YieldCPU();
	}	
}

void Signal(semaphore *S)
{
/*	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGALRM);
	
	if( sigprocmask( SIG_BLOCK , NULL , &set )  < 0 )
                printf("Line:%d  wait():Error to block the SIGALRM\n",__LINE__);
*/	
	S->value++;
	tcb_t *T = NULL;

	if( S->value <= 0 ){
		Pop(&T, &(S->list));
		deleteById(T->tid, &T, &suspendedQ);
		T->stat.state = READY;
		Push(T, &readyQ);
	}
	//ualarm( remtime, 0 );
	
/*	if( sigprocmask( SIG_UNBLOCK , NULL , &set )  < 0 )
                printf("Line:%d  wait():Error to unblock the SIGALRM\n",__LINE__);
*/
}




