/**
* Implement the short job first algorithm.
* Created by Chunyu Xue in May. 2020.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "task.h"
#include "list.h"
#include "cpu.h"
#include "schedulers.h"

#define Max 10000000

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
    struct node **tmp_min;							// tmp min
    tmp_min = malloc(sizeof(struct node *));
    struct node **prev_min;							// prev node of tmp_min
    prev_min = malloc(sizeof(struct node *));

    int turnaroundTime = 0;
    int waitTime = 0;		// Both wait time and response time in FCFS
    int min_burst;			// tmp min burst
    int cnt = 0;
    int cur_waitTime = 0;

    while((*head)->next != NULL){
        (*p) = (*head);
        (*prev_p) = (*p);
        (*tmp_min) = (*head);
        (*prev_min) = (*head);

        min_burst = (*head)->task->burst;
        while((*p)->next != NULL)
        {
            (*prev_p) = (*p);
            (*p) = (*p)->next;
            if((*p)->task->burst < min_burst){
                min_burst = (*p)->task->burst;
                (*prev_min) = (*prev_p);
                (*tmp_min) = (*p);
            }
        }
        /** Run the task */
        run((*tmp_min)->task, (*tmp_min)->task->burst);

        /** Update time information */
        cnt++;
        turnaroundTime += ((*tmp_min)->task->burst + cur_waitTime);
        waitTime += cur_waitTime;
        cur_waitTime += (*tmp_min)->task->burst;

        /** Print tid for the task */
        printf("The unique task identifier of task [%s] is: [%d]\n", (*tmp_min)->task->name, (*tmp_min)->task->tid);
        printf("----------------------------------------------\n");

        /** Delete the task */
        delete(head, (*tmp_min)->task);

        if((*tmp_min) != (*head)){		                // Delete the min node
            (*prev_min)->next = (*tmp_min)->next;
        }
        else{							                // If head node is the min, delete it and update the head
            (*head) = (*head)->next;
        }
    }
    /** For head */
    run((*head)->task, (*head)->task->burst);
    printf("The unique task identifier of task [%s] is: [%d]\n", (*tmp_min)->task->name, (*tmp_min)->task->tid);
    printf("----------------------------------------------\n");
    /** Update time information */
    cnt++;
    turnaroundTime += ((*tmp_min)->task->burst + cur_waitTime);
    waitTime += cur_waitTime;

    /** Print turnaround time, waiting time and response time. */
    printf("The average turnaround time is: %f units\n", ((double)turnaroundTime / (double)cnt));
    printf("The average wait time is: %f units\n", ((double)waitTime / (double)cnt));
    printf("The average response time is: %f units\n", ((double)waitTime / (double)cnt));

    /** Free space */
    free(p);
    free(prev_p);
    free(tmp_min);
    free(prev_min);
}
