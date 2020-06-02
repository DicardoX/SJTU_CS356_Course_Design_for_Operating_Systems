/**
* Implement the priority algorithm.
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
    struct node **prev_p;
    prev_p = malloc(sizeof(struct node *));
    struct node **tmp_max;							// tmp min
    tmp_max = malloc(sizeof(struct node *));
    struct node **prev_max;							// prev node of tmp_min
    prev_max = malloc(sizeof(struct node *));

    int turnaroundTime = 0;
    int waitTime = 0;		// Both wait time and response time in FCFS
    int max_priority;			// tmp min burst
    int cnt = 0;
    int cur_waitTime = 0;

    while((*head)->next != NULL){
        (*p) = (*head);
        (*prev_p) = (*p);
        (*tmp_max) = (*head);
        (*prev_max) = (*head);

        max_priority = (*head)->task->priority;
        while((*p)->next != NULL)
        {
            (*prev_p) = (*p);
            (*p) = (*p)->next;
            if((*p)->task->priority > max_priority){
                max_priority = (*p)->task->priority;
                (*prev_max) = (*prev_p);
                (*tmp_max) = (*p);
            }
        }
        /** Run the task */
        run((*tmp_max)->task, (*tmp_max)->task->burst);

        /** Update time information */
        cnt++;
        turnaroundTime += ((*tmp_max)->task->burst + cur_waitTime);
        waitTime += cur_waitTime;
        cur_waitTime += (*tmp_max)->task->burst;

        /** Print tid for the task */
        printf("The unique task identifier of task [%s] is: [%d]\n", (*tmp_max)->task->name, (*tmp_max)->task->tid);
        printf("----------------------------------------------\n");

        /** Delete the task */
        delete(head, (*tmp_max)->task);

        if((*tmp_max) != (*head)){      // Delete the min node
            (*prev_max)->next = (*tmp_max)->next;
        }
        else{                           // If head node is the min, delete it and update the head
            (*head) = (*head)->next;
        }
    }
    /** For head */
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
    free(prev_p);
    free(tmp_max);
    free(prev_max);
}
