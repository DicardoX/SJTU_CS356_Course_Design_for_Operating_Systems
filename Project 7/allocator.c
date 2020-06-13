#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/** Max amount of memory */
#define MAX_SIZE 10485760
/** Max amount of blocks */
#define MAX_AMOUNT 1000
/** Max length per command */
#define MAX_LENGTH 50

typedef struct{
    int status;         // 1 for allocated, 0 for unused
    int start;
    int end;
    char *name;
} Block;

/** Memory array */
int memory[MAX_SIZE];
/** Memory size */
int memSize;
/** Block array */
Block block[MAX_AMOUNT];
/** Block size */
int blockSize;

/** Initialization */
void init(int argc, char *argv[])
{
    memSize = atoi(argv[1]);
    blockSize = 0;
    block[blockSize].start = 0;
    block[blockSize].end = memSize;
    block[blockSize].status = 0;        // Unused
    block[blockSize].name = "Unused";
    blockSize++;
}

/** Process the string */
char *stringProcess(char *buffer)
{
    char *tmp = malloc(sizeof(char) * MAX_LENGTH);
    int i = 0, j = 0, last = 0;
    for( ; i < MAX_LENGTH; i++){
        if(buffer[i] == '\n')
            break;
        if(buffer[i] != ' '){
            last = i;
            tmp[j++] = buffer[i];
        }
        else if(i - last == 1 && j != 0){
            tmp[j++] = buffer[i];
        }
    }
    if(tmp[j-1] == ' ')
        tmp[j-1] = '\n';
    else
        tmp[j] = '\n';
    free(buffer);
    buffer = tmp;
    return buffer;
}

void compact()
{
    if(blockSize == 1){
        return;
    }

    Block tmp_block[MAX_AMOUNT];
    int tmp_blockSize = 0;
    int tmp_vis = 0;
    for(int i = 0; i < blockSize; i++){
        if(block[i].status == 1){
            tmp_block[tmp_blockSize].start = tmp_vis;
            tmp_block[tmp_blockSize].end = tmp_vis + block[i].end - block[i].start;
            tmp_block[tmp_blockSize].status = block[i].status;
            tmp_block[tmp_blockSize].name = block[i].name;
            tmp_vis = tmp_block[tmp_blockSize].end + 1;          // New start
            tmp_blockSize++;
        }
    }
    /** Copy back */
    for(int i = 0; i < tmp_blockSize; i++){
        block[i].start = tmp_block[i].start;
        block[i].end = tmp_block[i].end;
        block[i].status = tmp_block[i].status;
        block[i].name = tmp_block[i].name;
    }
    /** Set unused block */
    blockSize = tmp_blockSize + 1;
    block[blockSize - 1].start = block[blockSize - 2].end + 1;
    block[blockSize - 1].end = memSize;
    block[blockSize - 1].status = 0;        // Unused
    block[blockSize - 1].name = "Unused";

    //printf("Successfully compact!\n");
}

/** Insert */
void insert(int size, char *name, int start, int end, int num)
{
    /** If size == block[num].size */
    if(size == block[num].end - block[num].start + 1){
        block[num].status = 1;      // Allocated
        block[num].name = name;
        return;
    }

    blockSize++;
    /** Shift */
    for(int i = blockSize - 1; i > num + 1; i--){
        block[i].start = block[i-1].start;
        block[i].end = block[i-1].end;
        block[i].status = block[i-1].status;
        block[i].name = block[i-1].name;
    }
    /** Split */
    /// Right half
    block[num+1].end = block[num].end;
    block[num+1].name = block[num].name;
    block[num+1].status = block[num].status;
    block[num+1].start = block[num].start + size;
    /// Left half
    block[num].start = block[num].start;
    block[num].end = block[num].start + size - 1;
    block[num].status = 1;      // Allocated
    block[num].name = name;

}

/** Merge */
void merge(int start, int end, int num)
{
    /** Shift */
    for(int i = num; i < blockSize - 1; i++){
        block[i].start = block[i+1].start;
        block[i].end = block[i+1].end;
        block[i].status = block[i+1].status;
        block[i].name = block[i+1].name;
    }
    /** Merge */
    block[num-1].start = start;
    block[num-1].end = end;
    block[num-1].status = 0;        // Unused
    block[num-1].name = "Unused";

    blockSize--;
}

void request(char *buffer)
{
    int size;
    char type;
    char *name = malloc(sizeof(char) * MAX_LENGTH);
    sscanf(buffer, "%s %d %c", name, &size, &type);
    /** Flag for allocated */
    int isAllocated = 0;
    /** Best fit block */
    int bNum = -1, bSize = MAX_SIZE;
    /** Worst fit block */
    int wNum = -1, wSize = 0;

    /** Operation */
    switch (type) {
        case 'F':       // First hit
            for(int i = 0; i < blockSize; i++){
                if(block[i].status == 0 && block[i].end - block[i].start + 1 >= size){
                    insert(size, name, block[i].start, block[i].end, i);
                    isAllocated = 1;
                    break;
                }
            }
            break;
        case 'B':
            for(int i = 0; i < blockSize; i++){
                if(block[i].status == 0 && block[i].end - block[i].start + 1 >= size && block[i].end - block[i].start + 1 < bSize){
                    bNum = i;
                    bSize = block[i].end - block[i].start + 1;
                }
            }
            if(bNum != -1){
                insert(size, name, block[bNum].start, block[bNum].end, bNum);
                isAllocated = 1;
            }
            break;
        case 'W':
            for(int i = 0; i < blockSize; i++){
                if(block[i].status == 0 && block[i].end - block[i].start + 1 >= size && block[i].end - block[i].start + 1 > wSize){
                    wNum = i;
                    wSize = block[i].end - block[i].start + 1;
                }
            }
            if(wNum != -1){
                insert(size, name, block[wNum].start, block[wNum].end, wNum);
                isAllocated = 1;
            }
            break;
        default:
            printf("Error: wrong allocating type...\n");
            return;
    }
    if(!isAllocated)
        printf("Error: could not find a suitable hole to insert...\n");
    //else
        //printf("Successfully insert!\n");
}

void release(char *buffer)
{
    /** Get name */
    char *name = malloc(sizeof(char) * MAX_LENGTH);
    int idx = 0;
    for( ; idx < MAX_LENGTH && buffer[idx] != ' ' && buffer[idx] != '\n'; idx++) ;
    strncpy(name, buffer, idx);
    /** Operation */
    int name_existed = 0;
    int num = 0;
    for(int i = 0; i < blockSize; i++){
        if(strcmp(name, block[i].name) == 0){
            name_existed = 1;
            num = i;
            break;
        }
    }
    if(!name_existed){
        printf("Error: name does not exist...\n");
        return;
    }
    /** Clean block */
    block[num].status = 0;
    block[num].name = "Unused";
    /** Merge */
    /// Left
    if(num > 0 && block[num-1].status == 0){
        merge(block[num-1].start, block[num].end, num);
        num--;
    }
    /// Right
    if(num < blockSize - 1 && block[num+1].status == 0){
        merge(block[num].start, block[num+1].end, num+1);
    }
    //printf("Successfully release %s!\n", name);
}

void statusDisplay()
{
    for(int i = 0; i < blockSize; i++){
        printf("Addresses [%d : %d] Process %s\n", block[i].start, block[i].end, block[i].name);
    }

}

int main(int argc, char *argv[])
{
    /** Input error */
    if(argc != 2){
        printf("Error: wrong initial command arguments...exit\n");
        exit(1);
    }

    /** Initialization */
    init(argc, argv);

    /** Running */
    char *buffer = malloc(sizeof(char) * MAX_LENGTH);
    while(1)
    {
        printf("allocator>");
        fgets(buffer, MAX_LENGTH, stdin);
        buffer = stringProcess(buffer);
        /** Exit */
        if(buffer[0] == 'X'){
            break;
        }
        /** Compact */
        if(buffer[0] == 'C'){
            compact();
            continue;
        }
        /** Request */
        if(strncmp(buffer, "RQ", 2) == 0){
            request(buffer + 3);        // Remove "RQ "
            continue;
        }
        /** Release */
        if(strncmp(buffer, "RL", 2) == 0){
            release(buffer + 3);        // Remove "RL "
            continue;
        }
        /** Display status */
        if(strncmp(buffer, "STAT\n", 2) == 0){
            statusDisplay();
        }
        /** Error command, print message */
        //printf("Error command, please try again...\n");
    }

    return 0;
}
