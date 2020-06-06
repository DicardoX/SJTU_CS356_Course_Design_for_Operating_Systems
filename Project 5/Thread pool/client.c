/**
 * Example client program that uses thread pool.
 */

#include <stdio.h>
#include <unistd.h>
#include "threadpool.h"
#include <stdlib.h>

#define MOD1 1000
#define MOD2 2000

struct data
{
    int a;
    int b;
};

void add(void *param)
{
    struct data *temp;
    temp = (struct data*)param;

    printf("I add two values %d and %d result = %d\n",temp->a, temp->b, temp->a + temp->b);
    //printf("----------------Thread operation succeed!-----------------\n");
}

int main(void)
{
    // create some work to do
    struct data work[MAX_TASK];
    
    for(int i=0; i < MAX_TASK; i++){
    	work[i].a = rand()%MOD1;
    	work[i].b = rand()%MOD2;
    }

    // initialize the thread pool
    pool_init();
    
    sleep(1);

    // submit the work to the queue
    for(int i=0; i < MAX_TASK; i++)
    	pool_submit(&add, &work[i]);
    	

    // may be helpful 
    sleep(3);

	printf("aaaa\n");

    pool_shutdown();

    return 0;
}
