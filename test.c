#include "thread.h"

int fid,gid,hid,ret;
void *arg;

void* f(void *);
void g(void);
void h(void);
//void* p(void*);
int main()
{
  
	char msg = 'F';
  	fid=CreateThreadWithArgs(f, &msg);
  	gid=CreateThread(g);
  	hid=CreateThread(h);
  	Go();
return 0;
}



void* f(void* j)
{
  	int i=0,k;
  //int ret;
  	status_t stat;
  	printf("Got argument value : %c \n",*(char *)j);
  	while(1)
	{
    		printf("in f: %d\n",i++);
     	//	if(i==3)
     // 	{
       //		GetStatus(hid,&stat);
       //		print_stat(stt);
      //  		SuspendThread(fid);
    // 		}
       		if (i==3)
		{
       			ResumeThread(hid);
			break;
		}
	    	usleep(SECOND);
  	}
  
  	ret= i+38;
return &ret;
}

void g(void)
{
  	int i=0,k;
	//void *arg;
  	while(1)
	{
    		printf("in g: %d\n",i++);
   		if(9==i)
   		{
    			arg=GetThreadResult(fid);
    			printf("Got return value from thread %d : %d\n",fid,*(int *)arg);
  	
			SleepThread(2);
    		}
    		if(i==17)
    			CleanUp();
    		usleep(SECOND/2);
  	}
}

void h(void)
{
  	int i=0,j,k;
  	while(1)
	{

    		printf("in h: %d\n",i++);
    		if(i==3)
    		{
    			SuspendThread(GetMyId());
    			//YieldCPU();
			//SleepThread(GetMyId());
			//SleepThread(gid);
    		}
    		if(i==20)
    		{
    			//SuspendThread(GetMyId());
    			//YieldCPU();
    			break;
    		}
    		usleep(SECOND/4);
  	}
}


