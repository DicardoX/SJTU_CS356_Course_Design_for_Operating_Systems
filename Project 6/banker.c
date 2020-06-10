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


/** The available amount of each resource */
int *available;
/** The maximum demand of each customer */
int **maximum;
/** The amount currently allocated to each customer */
int **allocation;
/** The remaining need of each customer */
int **need;

/** The number of customer and resources types */
int customersNum = 0;
int resourcesTypeNum = 0;

int* parseLine(char *buffer){
    int i, j=0;
    int *arr = malloc(sizeof(int) * (resourcesTypeNum+1));
    /* get the customer index */
    arr[j++] = (int)(buffer[0]) - (int)('0');

    for(i=2;i<MAX_LENGTH;i++){
        if(buffer[i] == ' ' || buffer[i] == '\n')
            arr[j++] = (int)(buffer[i-1]) - (int)('0');
        if(buffer[i] == '\n')
            break;
    }
    if(j != resourcesTypeNum+1){
        printf("Error: you should match the number of resources...\n");
    }
    return arr;
}

char *preprocess(char *buffer){
    int i,j=0;
    int find, last;
    char *tmp = malloc(sizeof(char) * MAX_LENGTH);
    last = 0;
    for(i=0;i<MAX_LENGTH;i++){
        if(buffer[i] == '\n')   break;
        if(buffer[i] != ' '){
            last = i;
            tmp[j++] = buffer[i];
        }else if(i-last == 1){
            tmp[j++] = buffer[i];
        }
    }
    if(tmp[j-1] == ' ') tmp[j-1] = '\n';
    else tmp[j] = '\n';
    free(buffer);
    return tmp;
}

/** Release the resources for a specific customer */
int releaseResources(int customerIndex, int *strategy){
    int flag = 1;
    for(int i = 0; i < resourcesTypeNum; i++){
        if(allocation[customerIndex][i]<strategy[i]){
            flag = 0;
        }
    }
    if(flag){
        for(int i = 0; i < resourcesTypeNum; i++){
            allocation[customerIndex][i] -= strategy[i];
            need[customerIndex][i] += strategy[i];
            available[i] += strategy[i];
        }
    }
    return flag;
}

/** Get the resources of each type */
void getResources(int argc, char *argv[]){
    if(argc != resourcesTypeNum + 1){
        printf("Error! Your number of input isn't equal to the number of resources!\n");
        exit(1);
    }
    for(int i = 0; i < resourcesTypeNum; i++)
        available[i] = atoi(argv[i+1]);
}

/** Release the resources */
void handleReleases(char *buffer){
    int *arr = parseLine(buffer+3);         // Striping the "RL "
    int customerIndex = arr[0];
    if(releaseResources(customerIndex, arr+1))
        printf("Successfully release resources...\n");
    else
        printf("Sorry, customer doesn't have that much resources...\n");
}

/** Allocate the resources to a specific customer */
void allocateResources(int customerIndex, int *strategy){
    for(int i = 0; i < resourcesTypeNum; i++){
        allocation[customerIndex][i] += strategy[i];
        need[customerIndex][i] -= strategy[i];
        available[i] -= strategy[i];
    }
}

int findSomeCustomer(int *finished){
    int i, j;
    for(i = 0;i < customersNum; i++){
        for(j = 0;j < resourcesTypeNum; j++){
            if(need[i][j] > available[j])   break;
        }
        if(j == resourcesTypeNum && !finished[i])  return i;
    }
    return -1;
}

int checkSafe(int *finished){
    int i;
    for(i=0;i<customersNum && finished[i];i++) ;
    if(i == customersNum)  return 1;

    int customerIndex = findSomeCustomer(finished);
    if(customerIndex == -1)   return 0;
    else{
        int *strategy = malloc(sizeof(int) * resourcesTypeNum);
        memcpy(strategy, allocation[customerIndex], sizeof(int) * resourcesTypeNum);
        releaseResources(customerIndex, strategy);
        finished[customerIndex] = 1;
        checkSafe(finished);
    }
}

int checkSafeWrapper(){
    int flag;
    int *finished = malloc(sizeof(int) * customersNum);

    int **_maximum;
    int **_need;
    int *_available;
    int **_allocation;
    _maximum = malloc(sizeof(int *) * customersNum);
    _need = malloc(sizeof(int *) * customersNum);
    _allocation = malloc(sizeof(int *) * customersNum);
    _available = malloc(sizeof(int) * resourcesTypeNum);

    for(int i = 0; i < customersNum; i++){
        _maximum[i] = malloc(sizeof(int) * resourcesTypeNum);
        _need[i] = malloc(sizeof(int) * resourcesTypeNum);
        _allocation[i] = malloc(sizeof(int) * resourcesTypeNum);
        memcpy(_maximum[i], maximum[i], resourcesTypeNum * sizeof(int));
        memcpy(_need[i], need[i], resourcesTypeNum * sizeof(int));
        memcpy(_allocation[i], allocation[i], resourcesTypeNum * sizeof(int));

        /** Initialize bool array */
        finished[i] = 0;
    }
    memcpy(_available, available, sizeof(int) * resourcesTypeNum);

    /** Check safety */
    flag = checkSafe(finished);

    /** Recover and free */
    free(maximum);
    maximum = _maximum;
    free(need);
    need = _need;
    free(allocation);
    allocation = _allocation;
    free(available);
    available = _available;

    return flag;
}

/** Check whether the resources requested exceed the need limitation */
int judgeLegal(int customerIndex, int *strategy){
    for(int i = 0; i < resourcesTypeNum; i++){
        if(need[customerIndex][i] < strategy[i]){
            printf("Resource %d needs %d, but you've requested %d...\n", i+1, need[customerIndex][i], strategy[i]);
            return 0;
        }
    }
    return 1;
}

int handleRequest(char *buffer){
    int flag = 1;
    int *arr = parseLine(buffer+3);
    int customerIndex = arr[0];

    int legal_flag = judgeLegal(customerIndex, arr+1);
    if(!legal_flag){
        printf("Error: illegal input, Check your need...\n");
        return 0;
    }
    allocateResources(customerIndex, arr+1);

    if(!checkSafeWrapper()){
        flag = 0;
        releaseResources(customerIndex, arr+1);
    }
    if(flag)
        printf("Request has been satisfied...\n");
    else
        printf("Sorry, since your request may cause unsafe state, denied...\n");
    return flag;
}

void init(){
    char *buffer;
    int i,j,idx;
    FILE *fp = fopen("./test.txt", "r");
    assert(fp);

    /* do not use the fscanf, which will assume
     * that we know the number of resources beforehand*/
    buffer = malloc(sizeof(char) * MAX_LENGTH);
    while(fgets(buffer, MAX_LENGTH, fp) != NULL){
        /** First time */
        if(resourcesTypeNum == 0){
            for(i=0;i<MAX_LENGTH;i++){
                if(buffer[i] == ',' || buffer[i] == '\n')    resourcesTypeNum++;
                if(buffer[i] == '\n')   break;
            }
        }
        customersNum++;
    }
    printf("Totally %d customers and %d resources types\n", customersNum, resourcesTypeNum);
    maximum = malloc(sizeof(int *) * customersNum);
    allocation = malloc(sizeof(int *) * customersNum);
    need = malloc(sizeof(int *) * customersNum);
    available = malloc(sizeof(int) * resourcesTypeNum);

    for(i = 0; i < customersNum; i++){
        maximum[i] = malloc(sizeof(int) * resourcesTypeNum);
        allocation[i] = malloc(sizeof(int) * resourcesTypeNum);
        need[i] = malloc(sizeof(int) * resourcesTypeNum);
        memset(allocation[i], 0, resourcesTypeNum);
    }
    memset(available, 0, resourcesTypeNum);

    /* move back to the beginning */
    fseek(fp, 0, SEEK_SET);
    i = j = 0;
    while(fgets(buffer, MAX_LENGTH, fp) != NULL){
        for(idx=0;idx<MAX_LENGTH;idx++){
            if(buffer[idx] == ',' || buffer[idx] == '\n'){
                need[i][j] = atoi((char *)&buffer[idx-1]);
                maximum[i][j] = need[i][j];
                j++;
            }
            if(buffer[idx] == '\n') break;
        }
        i++; j=0;
    }
    fclose(fp);
}

/* output all the arrays */
void Output(){
    int i,j;
    /** Maximum matrix */
    printf("The maximum array is as follows:\n");
    for(i = 0;i < customersNum; i++){
        for(j = 0;j < resourcesTypeNum; j++)
            printf("%d ", maximum[i][j]);
        printf("\n");
    }
    /** Allocation matrix */
    printf("The allocation array is as follows:\n");
    for(i = 0; i < customersNum; i++){
        for(j = 0;j < resourcesTypeNum; j++)
            printf("%d ", allocation[i][j]);
        printf("\n");
    }
    /** Need matrix */
    printf("The need array is as follows:\n");
    for(i = 0;i < customersNum; i++){
        for(j = 0;j < resourcesTypeNum; j++)
            printf("%d ", need[i][j]);
        printf("\n");
    }
    /** Available array */
    printf("The available array is as follows:\n");
    for(i = 0;i < resourcesTypeNum; i++)
        printf("%d ", available[i]);
    printf("\n");
}

int main(int argc, char *argv[]){
    char *buffer;
    /* we will use the buffer to store the input from stdin */
    buffer = malloc(sizeof(char) * MAX_LENGTH);
    init();
    /* get the resources of each type */
    getResources(argc, argv);
    if(!checkSafeWrapper()) printf("Warning: current state is unsafe.\n");
    while(1){
        printf("Please input your command:\n");
        printf("osh>");
        fgets(buffer, MAX_LENGTH, stdin);
        buffer = preprocess(buffer);
        if(strcmp(buffer, "exit\n") == 0){
            break;
        }
        if(buffer[0] == '*'){
            /** Output arrays */
            Output();
            continue;
        }
        if(strncmp(buffer, "RL", 2) == 0){
            /** Release resources */
            handleReleases(buffer);
            continue;
        }
        if(strncmp(buffer, "RQ", 2) == 0){
            /** Request resources */
            handleRequest(buffer);
            continue;
        }
        printf("Error: invalid command...\n");
    }

    /** Free space */
    for(int i = 0; i < resourcesTypeNum; i++){
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
