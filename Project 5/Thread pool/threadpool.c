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
#define MAX_THREAD 5

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
    struct node* tmp = head;
    if(head->next == NULL){
        printf("List becomes empty...\n");
        finished = 1;				// Finish
    }
    head = head->next;
    free(tmp);
    return res;
}

// the worker thread in the thread pool
void *worker(void *param)
{
    // execute the task
    //execute(worktodo.function, worktodo.data);

    while(1){
        /** No available task */
        if(!shutDown && taskNum == 0){
            printf("Thread %d is waiting for task...\n", (int)pthread_self() % MOD + MOD);
            if(finished){
            	return;
            }
            sem_wait(&available_taskNum);
            
        }
        /** Shutdown */
        if(shutDown){
            printf("Thread %d is shutting down...\n", (int)pthread_self() % MOD + MOD);
            pthread_exit(0);
        }
        /** Running task */
        pthread_mutex_lock(&mutex);                     // Lock
        task myTask = dequeue();
        taskNum--;
        pthread_mutex_unlock(&mutex);                   // Unlock
        execute(myTask.function, myTask.data);          // Run task
        printf("Thread %d has finished a task...\n", (int)pthread_self() % MOD + MOD);

    }
}

/**
 * Executes the task provided to the thread pool
 */
void execute(void (*somefunction)(void *p), void *p)
{
    (*somefunction)(p);
}

/**
 * Submits work to the pool.
 */
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

// initialize the thread pool
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

// shutdown the thread pool
void pool_shutdown(void)
{
    printf("Shutting down...\n");
    /** Delete link list */
    struct node *tmp;
    while(head->next != NULL){
        tmp = head;
        head = head->next;
        free(tmp);
    }
    free(head);
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

}