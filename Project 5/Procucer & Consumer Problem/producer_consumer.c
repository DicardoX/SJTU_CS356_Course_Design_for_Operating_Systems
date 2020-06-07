#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

#define BUFFER_SIZE 10
#define MAX_CONSUMER 5
#define MAX_PRODUCER 5
#define MAX_SLEEP_TIME 10
#define MAX_ITEM_SIZE 1000
#define SUCCESS 0
#define FAILED -1

/** Semaphore */
sem_t empty, full;
/** Mutex */
pthread_mutex_t mutex;
/** Buffer item definition */
typedef int buffer_item;
/** Buffer array */
buffer_item buffer[BUFFER_SIZE];
/** Buffer pointer */
int pointer;
/** Thread definition */
pthread_t proThread[MAX_PRODUCER];
pthread_t conThread[MAX_CONSUMER];
pthread_attr_t pro_attr[MAX_PRODUCER];
pthread_attr_t con_attr[MAX_CONSUMER];
/** shutDown */
int is_shutdown;

void shutdown(int producerNum, int consumerNum)
{
    /** Set flag */
    is_shutdown = 1;
    /** Join thread */
    for(int i = 0; i < producerNum; i++)
        pthread_join(proThread[i], NULL);
    for(int i = 0; i < consumerNum; i++)
        pthread_join(conThread[i], NULL);
    /** Destroy mutex */
    pthread_mutex_destroy(&mutex);
    /** Destroy semaphore */
    sem_destroy(&full);
    sem_destroy(&empty);
}

/** Item insertion (return FAILED if error occurred, return SUCCESS if successfully operate) */
int insertItem(buffer_item item)
{

}

void *producer(){
    int rand_sleepTime;
    buffer_item item;
    srand((unsigned)time(NULL));
    while(!is_shutdown){
        rand_sleepTime = rand() % (MAX_SLEEP_TIME + 1);
        /** Randomly sleep */
        sleep(rand_sleepTime);

        /** Randomly produce item */
        item = rand() % (MAX_ITEM_SIZE + 1);
        if(insertItem(item) == FAILED){
            printf("Error occurred when inserting items into buffer...exit\n");
            exit(1);
        }

    }
}

int main(int argc, char *argv[]) {
    int producerNum, consumerNum;
    int sleepTime;

    /** Initialization */
    producerNum = 0;
    consumerNum = 0;
    sleepTime = 0;
    is_shutdown = 0;

    char cmd;
    while((cmd = (char)getopt(argc, argv, "p:c:s:")) != -1){
        switch (cmd) {
            case 'p':
                producerNum = atoi(optarg);
                break;
            case 'c':
                consumerNum = atoi(optarg);
                break;
            case 's':
                sleepTime = atoi(optarg);
                break;
            default:
                printf("Error occurred when input command...exit\n");
                exit(1);
        }

    }

    if(producerNum > MAX_PRODUCER){
        printf("Exceeded max number of producer...exit\n");
        exit(1);
    }
    if(consumerNum > MAX_CONSUMER){
        printf("Exceeded max number of consumer...exit\n");
        exit(1);
    }

    /** Thread attribution initialization */
    for(int i = 0; i < producerNum; i++){
        pthread_attr_init(&pro_attr[i]);
    }
    for(int i = 0; i < consumerNum; i++){
        pthread_attr_init(&con_attr[i]);
    }

    /** Thread creation */


    /** Sleep */
    sleep(sleepTime);

    /** Shutdown */
    shutdown(producerNum, consumerNum);

    return 0;
}
