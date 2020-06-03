/**
* Implement the priority round robin algorithm.
* Created by Chunyu Xue in May, 2020.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "task.h"
#include "list.h"
#include "cpu.h"
#include "schedulers.h"

#define TIME_PIECE 10
#define MAX 100000

extern struct node **head;
extern int tid_generator;

void add(char *name, int priority, int burst){
    Task *task;
    task = malloc(sizeof(Task));
    task->name = malloc(sizeof(char) * 10);

    strcpy(task->name, name);
    task->burst = burst;
    task->priority = priority;

    __sync_fetch_and_add(&tid_generator, 1);	// Use atomic integer to update tid_generator
    task->tid = tid_generator;

    insert(head, task);
}

void shift_to_end(struct node ** p){
    Task *task;
    task = malloc(sizeof(Task));
    task = (*p)->task;
    while((*p)->next != NULL){
        (*p)->task = (*p)->next->task;
        (*p) = (*p)->next;
    }
    (*p)->task = task;
}

void schedule(){
    struct node **headNew;
    headNew = malloc(sizeof(struct node *));
    (*headNew) = NULL;
    int turnaroundTime = 0;
    int waitTime = 0;
    int responseTime = 0;
    int cnt = 0;
    int cur_Time = 0;
    int is_visited[MAX];
    int total_burstTime = 0;


    /** reverse the list */
    struct node *newNode;
    while((*head) != NULL){
        cnt++;			// Update # of tasks
        newNode = malloc(sizeof(struct node));
        newNode->task = (*head)->task;
        newNode->next = *headNew;
        *headNew = newNode;
        *head = (*head)->next;
    }

    /** Initialize status array */
    for(int i=1; i <= cnt; i++)
        is_visited[i] = 0;


    /** priority_rr operation */
    struct node **tmp_max;
    tmp_max = malloc(sizeof(struct node *));
    struct node **p;
    p = malloc(sizeof(struct node *));

    (*p) = (*headNew);
    /** Calculate the sum of burst time */
    while((*p) != NULL){
        total_burstTime += (*p)->task->burst;
        (*p) = (*p)->next;
    }

    int prevTime = 0;
    while(*headNew != NULL){
        (*p) = (*headNew);
        int max_priority = 0;
        while((*p) != NULL){
            if((*p)->task->priority > max_priority){
                max_priority = (*p)->task->priority;
                (*tmp_max) = (*p);
            }
            (*p) = (*p)->next;
        }



        if((*tmp_max)->task->burst > TIME_PIECE){
            /** Update current time */
            cur_Time += TIME_PIECE;

            run((*tmp_max)->task, TIME_PIECE);
            printf("The unique task identifier of task [%s] is: [%d]\n", (*tmp_max)->task->name, (*tmp_max)->task->tid);
            printf("----------------------------------------------\n");
            (*tmp_max)->task->burst -= TIME_PIECE;

            shift_to_end(tmp_max);
        }
        else{
            /** Update current time and turnaround time */
            cur_Time += (*tmp_max)->task->burst;
            turnaroundTime += cur_Time;

            run((*tmp_max)->task, (*tmp_max)->task->burst);
            printf("The unique task identifier of task [%s] is: [%d]\n", (*tmp_max)->task->name, (*tmp_max)->task->tid);
            printf("----------------------------------------------\n");
            delete(headNew, (*tmp_max)->task);
        }


        /** Update response time information */
        if(is_visited[(*tmp_max)->task->tid] == 0){
            responseTime += prevTime;
            is_visited[(*tmp_max)->task->tid] = 1;
        }

        prevTime = cur_Time;
    }

    waitTime = turnaroundTime - total_burstTime;
    /** Print turnaround time, waiting time and response time. */
    printf("The average turnaround time is: %f units\n", ((double)turnaroundTime / (double)cnt));
    printf("The average wait time is: %f units\n", ((double)waitTime / (double)cnt));
    printf("The average response time is: %f units\n", ((double)responseTime / (double)cnt));

    /** Free space */
    free(p);
    free(tmp_max);
    free(headNew);
    free(newNode);
}
