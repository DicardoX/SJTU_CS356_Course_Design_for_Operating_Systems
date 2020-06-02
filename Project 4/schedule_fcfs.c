/**
* Implement the first come first service algorithm.
* Created by Chunyu Xue in May. 2020.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "task.h"
#include "list.h"
#include "cpu.h"
#include "schedulers.h"

extern struct node **head;
extern int tid_generator;

void add(char *name, int priority, int burst){
    Task *task;
    task = malloc(sizeof(Task));
    task->name = malloc(sizeof(char) * 10);
    
    strcpy(task->name, name);
    task->burst = burst;
    task->priority = priority;
    
    __sync_fetch_and_add(&tid_generator, 1);		// Use atomic integer to update tid_generator
    task->tid = tid_generator;
    
    insert(head, task);
}

void schedule(){
    struct node **p;
    p = malloc(sizeof(struct node *));
    int turnaroundTime = 0;
    int waitTime = 0;		// Both wait time and response time in FCFS
    int cnt = 0;
    int cur_waitTime = 0;
    
    while((*head)->next != NULL){
 	    (*p) = (*head);
 	    while((*p)->next != NULL)
 	 	    (*p) = (*p)->next;  
 	 	/** Run the task */
 	 	run((*p)->task, (*p)->task->burst);
 	 	/** Update time information */
 	 	cnt++;
 	 	turnaroundTime += ((*p)->task->burst + cur_waitTime);
 	 	waitTime += cur_waitTime;
 	 	cur_waitTime += (*p)->task->burst;
 	 	
 	 	
 	 	/** Print tid for the task */
 	 	printf("The unique task identifier of task [%s] is: [%d]\n", (*p)->task->name, (*p)->task->tid);
 	 	printf("----------------------------------------------\n");
 	 	/** Delete the task */
 	 	delete(head, (*p)->task);
    }
    run((*head)->task, (*head)->task->burst);
    printf("The unique task identifier of task [%s] is: [%d]\n", (*head)->task->name, (*head)->task->tid);
 	printf("----------------------------------------------\n");
 	/** Update time information */
 	cnt++;
 	turnaroundTime += ((*head)->task->burst + cur_waitTime);
 	waitTime += cur_waitTime;
    
    /** Print turnaround time, waiting time and response time. */
    printf("The average turnaround time is: %f units\n", ((double)turnaroundTime / (double)cnt));
    printf("The average wait time is: %f units\n", ((double)waitTime / (double)cnt));
    printf("The average response time is: %f units\n", ((double)waitTime / (double)cnt));
    
    /** Free space */
    free(p);
}
