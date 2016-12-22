#include "thread.h"

#define BUFFERSIZE 5

int pid,cid,count;
int BUFFER[BUFFERSIZE];
semaphore Empty,Full,Mutex;
struct timeval p,c;
struct timezone tz;

void producer();
void consumer();

int main(void)
{
    pid = CreateThread(producer);
    cid = CreateThread(consumer);
    semInit(&Empty, BUFFERSIZE);
    semInit(&Full, 0);
    semInit(&Mutex, 1);
    Go();

return 0;
}

void producer()
{
    int in=0,val;
    while(true)
    {
    
    //produce
	count++;
    	gettimeofday(&p,&tz);
        val = p.tv_usec%10;
        val++;
        
        Wait(&Empty);
        Wait(&Mutex);
       
    //put into buffer         
		BUFFER[in] = val;
		in = (in+1)%BUFFERSIZE;
		printf("Produced : %d\n",val);
		count++;
                        
        Signal(&Mutex);
        Signal(&Full);
        
        val = val%5;
        if(!val)
		val++;
        usleep(SECOND/val);
    }
}


void consumer()
{
    int out=0,val,sus;
    
    while(true)
    {
    
    	gettimeofday(&c,&tz);
        sus = c.tv_usec%5;
        
        Wait(&Full);
        Wait(&Mutex);

        //take out from buffer
            	val = BUFFER[out];
            	out = (out+1)%BUFFERSIZE;
            	count--;
		if(count == 0)
			break;
    
        Signal(&Mutex);
        Signal(&Empty);
 
        //consume
        printf("Consumed : %d\n",val);
        if (sus==0)
            	sus++;
        usleep(SECOND/sus);
    }
}
        

