/**
* Implement the round robin algorithm.
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
        cnt++;										// Update # of tasks
        newNode = malloc(sizeof(struct node));
        newNode->task = (*head)->task;
        newNode->next = *headNew;
        *headNew = newNode;
        *head = (*head)->next;
    }

    /** Initialize status array */
    for(int i=1; i <= cnt; i++)
        is_visited[i] = 0;


    /** rr operation */
    struct node **p;
    p = malloc(sizeof(struct node *));

    (*p) = (*headNew);
    /** Calculate the sum of burst time */
    while((*p) != NULL){
        total_burstTime += (*p)->task->burst;
        (*p) = (*p)->next;
    }

    (*p) = (*headNew);
    int prev_burstTime = 0;
    while(*headNew != NULL){

        /** Update response time information */
        if(is_visited[(*p)->task->tid] == 0){
            if((*p)->task->burst > TIME_PIECE){
                responseTime += prev_burstTime;
                prev_burstTime = TIME_PIECE;
            }
            else{
                responseTime += prev_burstTime;
                prev_burstTime = (*p)->task->burst;
            }
            is_visited[(*p)->task->tid] = 1;
        }


        if((*p)->task->burst > TIME_PIECE){
            /** Update current time */
            cur_Time += TIME_PIECE;

            run((*p)->task, TIME_PIECE);
            printf("The unique task identifier of task [%s] is: [%d]\n", (*p)->task->name, (*p)->task->tid);
            printf("----------------------------------------------\n");
            (*p)->task->burst -= TIME_PIECE;
            if((*p)->next == NULL)
                (*p) = (*headNew);
            else{
                (*p) = (*p)->next;
            }
        }
        else{
            /** Update current time and turnaround time */
            cur_Time += (*p)->task->burst;
            turnaroundTime += cur_Time;

            run((*p)->task, (*p)->task->burst);
            printf("The unique task identifier of task [%s] is: [%d]\n", (*p)->task->name, (*p)->task->tid);
            printf("----------------------------------------------\n");
            delete(headNew, (*p)->task);
            if((*p)->next == NULL)
                (*p) = (*headNew);
            else
                (*p) = (*p)->next;
        }
    }

    waitTime = turnaroundTime - total_burstTime;
    /** Print turnaround time, waiting time and response time. */
    printf("The average turnaround time is: %f units\n", ((double)turnaroundTime / (double)cnt));
    printf("The average wait time is: %f units\n", ((double)waitTime / (double)cnt));
    printf("The average response time is: %f units\n", ((double)responseTime / (double)cnt));

    /** Free space */
    free(p);
    free(headNew);
    free(newNode);
}
