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

#define Size 6
int *arr;
int flag1, flag2;           // Whether two sort threads have finished

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
        flag1 = 1;                                  // Finished
    }
    if(strcmp(threadName, "sortThread2") == 0){
        merge_sort(arr, mid+1, Size-1);
        flag2 = 1;                                  // Finished
    }
    if(strcmp(threadName, "mergeThread") == 0){
        while(flag1 != 1 || flag2 != 1){}           // Busy waiting until two sort threads finishes

        merge(arr, 0, Size-1);
    }

    pthread_exit(0);
}

int main(void) {

    /** Randomly initialize the array.
     *  If you want to input your own data, just cancel the random part and use the scanf().
     * */
     
    int times = 500;
    while(times--)
    {
        
    }
    arr = malloc(sizeof(int) * Size);
    //srand((unsigned)time(NULL));
    for(int i=0; i < Size; i++) {
        //arr[i] = rand() % MAXSIZE;
        scanf("%d", &arr[i]);
    }

    /** Initialize the flag */
    flag1 = 0;
    flag2 = 0;

    printf("The initial array is:\n");
    for(int i=0; i < Size; i++){
        printf("%d ", arr[i]);
    }
    printf("\n");

    // Thread identifier & attributions, tid1 and tid2 are sort threads, while tid3 is merge thread
    pthread_t tid[3];
    pthread_attr_t attr[3];

    for(int i=0; i < 3; i++)
        pthread_attr_init(&attr[i]);

    // Thread name
    char *threadName[3];
    threadName[0] = "sortThread1";
    threadName[1] = "sortThread2";
    threadName[2] = "mergeThread";

    // Initialize the thread attribution and thread creation
    pthread_create(&tid[0], &attr[0], func, threadName[0]);
    pthread_create(&tid[1], &attr[1], func, threadName[1]);
    pthread_create(&tid[2], &attr[2], func, threadName[2]);

    // Thread join
    pthread_join(tid[0], NULL);
    pthread_join(tid[1], NULL);
    pthread_join(tid[2], NULL);

    // Print result
    printf("The sorted array is:\n");
    for(int i=0; i < Size; i++){
        printf("%d ", arr[i]);
    }

    return 0;
}
