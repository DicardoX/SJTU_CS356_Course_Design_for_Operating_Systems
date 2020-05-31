/**
 * Created by Chunyu Xue in May, 2020.
 * Note: we use merge sort for sorting threads.
 */

#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAXSIZE 500
#define MAX 500000      // The max iteration times

#define Size 6
int *arr;
int flag1, flag2;                                       // Whether two sort threads have finished

void merge(int a[], int low, int high)
{
    int mid = (high + low) / 2;
    int tmp_arr[high - low + 1];

    int left = low, right = mid + 1;
    int vis = 0;

    while(left <= mid && right <= high)
    {
        if (a[left] > a[right]){
            tmp_arr[vis++] = a[right++];
        }
        else{
            tmp_arr[vis++] = a[left++];
        }
    }

    while(left <= mid){
        tmp_arr[vis++] = a[left++];
    }
    while(right <= high){
        tmp_arr[vis++] = a[right++];
    }

    for(int i=0; i < (high - low + 1); i++){
        a[low + i] = tmp_arr[i];
    }
}

void merge_sort(int a[], int low, int high)
{
    if(low >= high)
        return;
    int mid = (low + high) / 2;

    merge_sort(a, low, mid);
    merge_sort(a, mid+1, high);

    merge(a, low, high);
}

void *func(void *threadName)
{
    int mid = (Size - 1) / 2;
    if(strcmp(threadName, "sortThread1") == 0){
        merge_sort(arr, 0, mid);
        flag1 = 1;                                          // Finished
    }
    if(strcmp(threadName, "sortThread2") == 0){
        merge_sort(arr, mid+1, Size-1);
        flag2 = 1;                                          // Finished
    }
    if(strcmp(threadName, "mergeThread") == 0){
        while(flag1 != 1 || flag2 != 1){}                   // Busy waiting until two sort threads finishes
        merge(arr, 0, Size-1);
        flag1 = 0;
        flag2 = 0;                                          // Reset the flag
    }

    pthread_exit(0);
}

int main(void)
{
    int times = MAX;
    while(times--)                                          // Note: If you don't want to use loop to check correctness, just note it
    {
        /** Initialize the array (input or randomly generated) */
        arr = malloc(sizeof(int) * Size);
        srand((unsigned)time(NULL));
        for(int i=0; i < Size; i++) {
            /** Randomly generated, if not use, just note it */
            arr[i] = (rand() % MAXSIZE * 2) - MAXSIZE;      // Range: -500 ~ 500

            /** Input, if not use, just note it */
            //scanf("%d", &arr[i]);
        }
        printf("The initial array is:\n");
        for(int i=0; i < Size; i++){
            printf("%d ", arr[i]);
        }
        printf("\n");

        /** Initialize the flag */
        flag1 = 0;
        flag2 = 0;

        /** Thread identifier & attributions, tid1 and tid2 are sort threads, while tid3 is merge thread */
        pthread_t tid[3];
        pthread_attr_t attr[3];

        /** Initialize Thread attribution */
        for(int i=0; i < 3; i++)
            pthread_attr_init(&attr[i]);

        /** Thread name */
        char *threadName[] = {"sortThread1", "sortThread2", "mergeThread"};

        /** Thread creation */
        pthread_create(&tid[0], &attr[0], func, threadName[0]);
        pthread_create(&tid[1], &attr[1], func, threadName[1]);
        pthread_create(&tid[2], &attr[2], func, threadName[2]);

        /** Thread join */
        pthread_join(tid[0], NULL);
        pthread_join(tid[1], NULL);
        pthread_join(tid[2], NULL);

        /** Print result */
        printf("The sorted array is:\n");
        for(int i=0; i < Size; i++){
            printf("%d ", arr[i]);
        }
        printf("\n");

        /** Check correctness */
        for(int i=1; i < Size; i++){
            if(arr[i-1] > arr[i]){
                printf("Failed!\n");
                exit(1);
            }
        }
        printf("--------Round %d Success!--------\n", MAX - times);
    }

    return 0;
}
