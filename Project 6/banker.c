/**
 * Created by Chunyu Xue in May, 2020
 * Name: Banker's Algorithm
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#define MAX_LENGTH 20
#define IS_SAFE 1
#define UNSAFE 0


/** The available amount of each resource */
int *available;
/** The maximum demand of each customer */
int **maximum;
/** The amount currently allocated to each customer */
int **allocation;
/** The remaining need of each customer */
int **need;

/** Current number of customer and resources */
int customersNum = 0;
int resourcesTypeNum = 0;

void init(){
    char *buffer;
    /** Open file */
    FILE *fileName = fopen("test.txt", "r");
    assert(fileName);                   // In case of failing to open file

    /** Read file and initialize number */
    buffer = malloc(sizeof(char) * MAX_LENGTH);
    while(fgets(buffer, MAX_LENGTH, fileName) != NULL){
        /** For the first time */
        if(resourcesTypeNum == 0){
            for(int i = 0; i < MAX_LENGTH; i++){
                if(buffer[i] == ',')
                    resourcesTypeNum++;
                if(buffer[i] == '\n'){
                    resourcesTypeNum++;
                    break;
                }
            }
        }
        customersNum++;
    }
    printf("Totally %d customers and %d resources types\n", customersNum, resourcesTypeNum);
    /** Initialize matrix */
    maximum = malloc(sizeof(int *) * customersNum);
    allocation = malloc(sizeof(int *) * customersNum);
    need = malloc(sizeof(int *) * customersNum);
    available = malloc(sizeof(int) * resourcesTypeNum);

    for(int i = 0; i < customersNum; i++){
        maximum[i] = malloc(sizeof(int) * resourcesTypeNum);
        allocation[i] = malloc(sizeof(int) * resourcesTypeNum);
        need[i] = malloc(sizeof(int) * resourcesTypeNum);
        //memset(maximum[i], 0, resourcesTypeNum);
        memset(allocation[i], 0, resourcesTypeNum);
        //memset(need[i], 0, resourcesTypeNum);
    }
    memset(available, 0, resourcesTypeNum);
   
    /** Move to the beginning and read data */
    int i, j = 0;
    fseek(fileName, 0, SEEK_SET);

    while(fgets(buffer, MAX_LENGTH, fileName) != NULL)
    {
        for(int idx = 0; idx < MAX_LENGTH; idx++){
            if(buffer[idx] == ',' || buffer[idx] == '\n'){
                need[i][j] = atoi((char *)&buffer[idx-1]);
                maximum[i][j] = need[i][j];
                j++;
            }
            if(buffer[idx] == '\n')
                break;
        }
        i++;
        j = 0;
    }
    /** Close file */
    fclose(fileName);
    printf("Successfully close file...\n");
}

int release(int customerIdx, int *array){
    int flag = 1;
    for(int i = 0; i < resourcesTypeNum; i++){
        if(allocation[customerIdx][i] < array[i]){
            flag = 0;
        }
    }
    if(flag){
        for(int i = 0; i < resourcesTypeNum; i++){
            allocation[customerIdx][i] -= array[i];
            need[customerIdx][i] += array[i];
            available[i] += array[i];
        }
    }
    return flag;
}

int is_safe(int *finished)
{
    int idx = 0;
    while (idx < customersNum && finished[idx])
        idx++;
    if(idx == customersNum)
        return IS_SAFE;

    /** Find the proper customer */
    int customerIdx = -1;
    int j;
    for(int i = 0; i < customersNum; i++){
        for(j = 0; j < resourcesTypeNum; j++){
            if(need[i][j] > available[j])
                break;
        }
        if(j == resourcesTypeNum && !finished[i])
            customerIdx = i;
    }
    if(customerIdx == -1) {
        return UNSAFE;
    }
    else{
        int *tmp = malloc(sizeof(int) * resourcesTypeNum);
        memcpy(tmp, allocation[customerIdx], sizeof(int) * resourcesTypeNum);
        release(customerIdx, tmp);
        finished[customerIdx] = 1;
        is_safe(finished);
    }
}

int safe_checker()
{
    int *finished = malloc(sizeof(int) * customersNum);
    int **tmp_maximum = malloc(sizeof(int) * customersNum);
    int **tmp_need = malloc(sizeof(int) * customersNum);
    int **tmp_allocation = malloc(sizeof(int) * customersNum);
    int *tmp_available = malloc(sizeof(int) * resourcesTypeNum);
    for(int i = 0; i < customersNum; i++)
    {
        tmp_maximum[i] = malloc(sizeof(int) * resourcesTypeNum);
        tmp_need[i] = malloc(sizeof(int) * resourcesTypeNum);
        tmp_allocation[i] = malloc(sizeof(int) * resourcesTypeNum);
        memcpy(tmp_maximum[i], maximum[i], resourcesTypeNum * sizeof(int));
        memcpy(tmp_need[i], need[i], resourcesTypeNum * sizeof(int));
        memcpy(tmp_allocation[i], allocation[i], resourcesTypeNum * sizeof(int));

        /** Initialize bool array */
        finished[i] = 0;
    }
    memcpy(tmp_available, available, sizeof(int) * customersNum);

    int flag = is_safe(finished);
    /** Free and recover */
    free(maximum);
    free(need);
    free(allocation);
    free(available);
    maximum = tmp_maximum;
    need = tmp_need;
    allocation = tmp_allocation;
    available = tmp_available;

    return flag;
}

char *getString(char *buffer){
    int find, last, idx;
    char *tmp = malloc(sizeof(char) * MAX_LENGTH);
    last = 0;
    idx = 0;
    for(int i = 0; i < MAX_LENGTH; i++){
        if(buffer[i] == '\n')
            break;
        if(buffer[i] != ' ') {
            last = i;
            tmp[idx++] = buffer[i];
            continue;
        }
        if(last == i - 1){
            tmp[idx++] = buffer[i];
        }
    }
    if(tmp[idx-1] == ' ')
        tmp[idx-1] = '\n';
    free(buffer);
    return tmp;
}

void Output()
{
    /** Maximum matrix */
    printf("The maximum matrix is as follows:\n");
    for(int i = 0; i < customersNum; i++){
        for(int j = 0;j < resourcesTypeNum; j++){
            printf("%d ", maximum[i][j]);
        }
        printf("\n");
    }
    /** Allocation matrix */
    printf("The allocation matrix is as follows:\n");
    for(int i = 0; i < customersNum; i++){
        for(int j = 0;j < resourcesTypeNum; j++){
            printf("%d ", allocation[i][j]);
        }
        printf("\n");
    }
    /** Need matrix */
    printf("The need matrix is as follows:\n");
    for(int i = 0; i < customersNum; i++){
        for(int j = 0;j < resourcesTypeNum; j++){
            printf("%d ", need[i][j]);
        }
        printf("\n");
    }
    /** Available array */
    printf("The available array is as follows:\n");
    for(int i = 0;i < resourcesTypeNum; i++){
        printf("%d ", available[i]);
    }
    printf("\n");

}

int* parseLine(char *buffer){
    int i, j=0;
    int *arr = malloc(sizeof(int) * (resourcesTypeNum+1));
    /* get the customer index */
    arr[j++] = (int)(buffer[0]) - (int)('0');

    for(i=2;i<MAX_LENGTH;i++){
        if(buffer[i] == ' ' || buffer[i] == '\n')   arr[j++] = (int)(buffer[i-1]) - (int)('0');
        if(buffer[i] == '\n')   break;
    }
    if(j != resourcesTypeNum+1){
        fprintf(stderr, "Invalid input, it should match the number of resources.\n");
    }
    return arr;
}

void release_op(char *buffer)
{
    int *arr = parseLine(buffer + 3);
    int customerIdx = arr[0];
    if(release(customerIdx, arr+1)){
        printf("Successfully released...\n");
    }
    else{
        printf("Error occurred when releasing resources...\n");
    }
}

void request_op(char *buffer)
{
    int flag = 1;
    int *arr = parseLine(buffer + 3);
    int customerIdx = arr[0];

    /** Check whether the resources requested exceeds the need */
    int legal_flag = 1;
    for(int i = 0; i < resourcesTypeNum; i++){
        if(need[customerIdx][i] < (arr+1)[i])
            legal_flag = 0;
    }
    if(!legal_flag){
        printf("Error: Resources requested exceeds the need...exit\n");
        exit(1);
    }

    /** Allocate resources */
    for(int i = 0; i < resourcesTypeNum; i++){
        allocation[customerIdx][i] += (arr+1)[i];
        need[customerIdx][i] -= (arr+1)[i];
        available[i] -= (arr+1)[i];
    }

    /** If unsafe after allocating, we release it */
    if(!safe_checker()){
        flag = 0;
        release(customerIdx, arr+1);
    }

    if(flag){
        printf("Successfully handle the request...\n");
    }
    else{
        printf("Sorry, but your request will lead to unsafe state, deny...\n");
    }
}

int main(int argc, char *argv[]) {
    char *buffer;
    buffer = malloc(sizeof(char) * MAX_LENGTH);

    /** Initialization and get maximum/need matrix */
    init();
    /** Get available matrix */
    if(argc != resourcesTypeNum + 1){
        printf("Error: number of input arguments doesn't match the number of resources\n");
        exit(-1);
    }
    for(int i = 0; i < resourcesTypeNum; i++)
        available[i] = atoi(argv[i+1]);
    /** Check safe */
    if(!safe_checker()){
        printf("Error: current state is unsafe...\n");
        //exit(1);
    }
    /** Running */
    while(1)
    {
        printf("osh>");
        fgets(buffer, MAX_LENGTH, stdin);
        buffer = getString(buffer);
        if(!strcmp(buffer, "exit\n")){
            break;
        }
        if(buffer[0] == '*'){
            /** Print all arrays */
            Output();
            continue;
        }
        if(strncmp(buffer, "RL", 2) == 0){
            /** Release resources */
            release_op(buffer);
            continue;
        }
        if(strncmp(buffer, "RQ", 2) == 0){
            /** Request resources */
            request_op(buffer);
            continue;
        }
        printf("Input error, please try again...\n");
    }

    /** Free arrays */
    for(int i = 0; i < customersNum; i++){
        free(maximum[i]);
        free(allocation[i]);
        free(need[i]);
    }
    free(maximum);
    free(allocation);
    free(need);
    free(available);

    return 0;
}
