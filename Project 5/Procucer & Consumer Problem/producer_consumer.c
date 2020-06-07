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
int pro_pointer, con_pointer;
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
    /** Display rest items in buffer */
    if(pro_pointer == con_pointer){
        printf("No item remains in buffer when shutdown...\n");
    }
    else{
        int cnt = 0;
        buffer_item rest_buffer[BUFFER_SIZE];
        while(con_pointer != pro_pointer){
            rest_buffer[cnt++] = buffer[con_pointer];
            con_pointer = (con_pointer + 1) % BUFFER_SIZE;
        }
        printf("There are %d items remaining in the buffer:\n", cnt);
        for(int i = 0; i < cnt; i++){
            printf("%d ", rest_buffer[i]);
        }
        printf("\n");
    }
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
    /** Wait until the buffer has an available position */
    if(sem_wait(&empty) == -1){
        return FAILED;
    }
    /** Critical section */
    if(pthread_mutex_lock(&mutex) != 0) {    // Lock
        return FAILED;
    }
    buffer[pro_pointer] = item;
    pro_pointer = (pro_pointer + 1) % BUFFER_SIZE;
    pthread_mutex_unlock(&mutex);   // Unlock
    /** Add the full semaphore */
    if(sem_post(&full) == -1){
        return FAILED;
    }
    return SUCCESS;
}

buffer_item removeItem(){
    buffer_item res;
    /** Wait until the buffer has an item */
    if(sem_wait(&full) == -1){
        printf("Error occurred when removing item...exit\n");
        exit(1);
    }
    /** Critical section */
    pthread_mutex_lock(&mutex);     // Lock
    res = buffer[con_pointer];
    con_pointer = (con_pointer + 1) % BUFFER_SIZE;
    pthread_mutex_unlock(&mutex);   // Unlock
    /** Add an available slot to buffer */
    if(sem_post(&empty) == -1){
        printf("Error occurred when removing item...exit\n");
        exit(1);
    }
    return res;
}

void *consumer(void *param){
    int rand_sleepTime;
    int id = *(int *)param;
    buffer_item item;
    srand((unsigned) time(NULL));
    while(!is_shutdown){
        rand_sleepTime = rand() % (MAX_SLEEP_TIME + 1);
        /** Randomly sleep */
        sleep(rand_sleepTime);

        /** Remove item */
        if(!is_shutdown){
            item = removeItem();
            printf("Consumer %d removes item %d from buffer successfully in thread %d...\n", id, item, ((int)pthread_self() % 1000 + 1000));
        }
    }
}


void *producer(void *param){
    int rand_sleepTime;
    int id = *(int *)param;
    buffer_item item;
    srand((unsigned) time(NULL));
    while(!is_shutdown){
        rand_sleepTime = rand() % (MAX_SLEEP_TIME + 1);
        /** Randomly sleep */
        sleep(rand_sleepTime);

        /** Randomly produce item */
        if(!is_shutdown){
            item = rand() % (MAX_ITEM_SIZE + 1);
            if(insertItem(item) == FAILED){
                printf("Error occurred when inserting items into buffer...exit\n");
                exit(1);
            }
            printf("Producer %d inserts item %d successfully in thread %d...\n", id, item, ((int)pthread_self() % 1000 + 1000));
        }
    }
}

void message(){
    /** Message */
    printf("Please input # of producer (no more than %d), # of consumer (no more than %d) and sleep time of the main thread until shutdown.\n", MAX_PRODUCER, MAX_CONSUMER);
    printf("Command format: ./app -p [# of producer] -c [# of consumer] -s [sleep time]\n");
}

int main(int argc, char *argv[]) {
    int producerNum, consumerNum;
    int sleepTime;

    /** Initialization */
    producerNum = 0;
    consumerNum = 0;
    sleepTime = 0;
    is_shutdown = 0;
    pro_pointer = 0;
    con_pointer = 0;
    /** Semaphore initialization */
    sem_init(&empty, 0, BUFFER_SIZE);
    sem_init(&full, 0, 0);
    /** Mutex initialization */
    pthread_mutex_init(&mutex, NULL);

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
                message();
                exit(1);
        }

    }

    if(producerNum == 0 || consumerNum == 0){
        printf("The # of producer or consumer must be larger than 0, please try again...exit\n");
        message();
        exit(1);
    }

    if(producerNum > MAX_PRODUCER){
        printf("Exceeded max number of producer...exit\n");
        message();
        exit(1);
    }
    if(consumerNum > MAX_CONSUMER){
        printf("Exceeded max number of consumer...exit\n");
        message();
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
    for(int i = 0; i < producerNum; i++){
        pthread_create(&proThread[i], &pro_attr[i], producer, (void *)&i);
    }
    for(int i = 0; i < consumerNum; i++){
        pthread_create(&conThread[i], &con_attr[i], consumer, (void *)&i);
    }

    /** Sleep */
    sleep(sleepTime);

    /** Shutdown */
    shutdown(producerNum, consumerNum);

    return 0;
}
