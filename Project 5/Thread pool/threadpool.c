/**
 * Implementation of thread pool.
 */

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include "threadpool.h"
#include <sys/types.h>
#include <unistd.h>

#define QUEUE_SIZE 10
#define NUMBER_OF_THREADS 3
#define MOD 1000

#define TRUE 1
#define ERROR 0
#define MAX_THREAD 3

/** This represents work that has to be completed by a thread in the pool */
typedef struct
{
    void (*function)(void *p);
    void *data;
} task;

/** Define node struct for the link list */
struct node
{
    task myTask;
    struct node* next;
};

/** Thread array */
pthread_t pid[MAX_THREAD];
pthread_attr_t attr[MAX_THREAD];
/** Head node */
struct node *head;
/** Mutex */
pthread_mutex_t mutex;
/** Flag for shutdown */
int shutDown;
/** Flag for finish */
int finished;
/** # of task */
int taskNum;
/** Semaphore */
sem_t available_taskNum;

/** Insert a task into the queue */
int enqueue(task t)
{
    struct node *newNode;
    struct node *tmp;
    newNode = malloc(sizeof(struct node));
    if(newNode == NULL){
        return ERROR;
    }
    newNode->myTask = t;
    newNode->next = NULL;

	if(head == NULL){
		head = newNode;
	}
	else{
		tmp = head;
    	while(tmp->next != NULL)
        	tmp = tmp->next;
    	tmp->next = newNode;
	}

    return TRUE;
}

/** Remove a task from the queue */
task dequeue()
{
    task res = head->myTask;
    struct node *tmp = head;
    if(head->next == NULL){
    	if(!finished)
        	printf("List becomes empty...\n");
        return res;
    }
    head = head->next;				// If head->next == NULL and execute to here, will cause segment fault !!!
    free(tmp);					// Delete used node
    return res;
}

/** the Worker threads in the thread pool */
void *worker(void *param)
{    

    while(shutDown != 1){
        /** No available task */
        if(!shutDown && taskNum <= 0){
            printf("Thread %d is waiting for task...\n", (int)pthread_self() % MOD + MOD);
            sem_wait(&available_taskNum);
        }
        /** Running task */
        pthread_mutex_lock(&mutex);                     // Lock
        task myTask = dequeue();
        if(taskNum > -1)
        	taskNum--;										// will be finally fixed to -1

        if(taskNum == 0){
        	finished = 1;
        	pthread_mutex_unlock(&mutex); 
        	execute(myTask.function, myTask.data);          // Run the last task
        	printf("Thread %d has finished a task...\n", (int)pthread_self() % MOD + MOD);
        	continue;
        } 
        if(!finished){
        	execute(myTask.function, myTask.data);          // Run task
        	printf("Thread %d has finished a task...\n", (int)pthread_self() % MOD + MOD);
        }  
        pthread_mutex_unlock(&mutex);                   // Unlock
        sleep(1);										// To make the threads could run task in turn, the thread MAY be in this sleep and doesn't print "waiting..." in the last
    }
    
    pthread_exit(0);
}

 /** Executes the task provided to the thread pool */
void execute(void (*somefunction)(void *p), void *p){
    (*somefunction)(p);    
}

/** Submits work to the pool */
int pool_submit(void (*somefunction)(void *p), void *p)
{
    task myTask;
    myTask.function = somefunction;
    myTask.data = p;
    pthread_mutex_lock(&mutex);         // Lock
    if(enqueue(myTask) == ERROR){       // Enqueue
        printf("Error occurred when task enqueue...\n");
        exit(1);
    }
    taskNum++;
    sem_post(&available_taskNum);
    pthread_mutex_unlock(&mutex);       // Unlock
    //printf("Successfully submit the task!\n");

    return 0;
}

/** Initialize the thread pool */
void pool_init(void)
{
    shutDown = 0;
    finished = 0;
    taskNum = 0;
    head = NULL;

    /** Thread initialization */
    for(int i=0; i < MAX_THREAD; i++){
        pthread_attr_init(&attr[i]);
        if(pthread_create(&pid[i], &attr[i], worker, NULL) != 0){
            printf("the %d -th thread failed to create...exit", i);
            exit(1);
        }
    }
    /** Set mutex */
    pthread_mutex_init(&mutex, NULL);
    /** Set semaphore */
    sem_init(&available_taskNum, 0, 0);

}

/** Shutdown the thread pool */
void pool_shutdown(void)
{
    printf("Shutting down...\n");
    	
    /** Set flag */
    shutDown = 1;
    /** Join thread */
    for(int i=0; i < MAX_THREAD; i++)
        sem_post(&available_taskNum);           // make thread shut down
    for(int i=0; i < MAX_THREAD; i++)
        pthread_join(pid[i], NULL);
    /** Delete mutex */
    pthread_mutex_destroy(&mutex);
    /** Delete semaphore */
    sem_destroy(&available_taskNum);
    
    /** Delete head node of the list */
    free(head);
	
	printf("Complete shutting down!\n");
}
