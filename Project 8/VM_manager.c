/**
 * Created by Chunyu Xue in May, 2020
 * Name: Virtual Machine Manager
 * */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


/** Maximum length of a line in the file */
#define MAX_LENGTH 20
/** Bit definition */
#define PAGE_NUMBER_BITS 8
#define FRAME_NUMBER_BITS 8
#define PAGE_OFFSET_BITS 8
#define FRAME_OFFSET_BITS 8
/** TLB */
#define TLB_SIZE 16
#define MAX_LRU 1000
#define NOT_FOUND -1

typedef struct{
    int pageNum;
    int frameNum;
    int lruAge;
} entry;

/** Size definition */
int pageSize, frameSize, numOfPage, numOfFrame;
/** Page table definition */
int *pageTable;
int pageFault;
/** Physical memory */
char **physicalMemory;
/** Memory LRU */
int *memLRU;
/** Flag for free frames */
int *frame_is_free;
/** TLB */
entry *tlb;
int tlbHit;
/** Number of access */
int accessNum;


void init()
{
    /** Size initialization */
    pageSize = frameSize = numOfFrame = numOfPage = 1;
    for(int i = 0; i < PAGE_NUMBER_BITS; i++)
        numOfPage = numOfPage << 1;
    for(int i = 0; i < FRAME_NUMBER_BITS; i++)
        numOfFrame = numOfFrame << 1;
    for(int i = 0; i < PAGE_OFFSET_BITS; i++)
        pageSize = pageSize << 1;
    for(int i = 0; i < FRAME_OFFSET_BITS; i++)
        frameSize = frameSize << 1;

    /** Page table initialization */
    pageTable = malloc(sizeof(int) * numOfPage);
    for(int i = 0; i < numOfPage; i++){
        pageTable[i] = NOT_FOUND;
    }
    pageFault = 0;
    /** Memory initialization */
    physicalMemory = malloc(sizeof(char *) * numOfFrame);
    for(int i = 0; i < numOfFrame; i++)
        physicalMemory[i] = malloc(sizeof(char) * frameSize);
    /** Memory LRU initialization */
    memLRU = malloc(sizeof(int) * numOfFrame);
    for(int i = 0; i < numOfFrame; i++)
        memLRU[i] = MAX_LRU;
    /** Flag for free frames initialization */
    frame_is_free = malloc(sizeof(int) * numOfFrame);
    for(int i = 0; i < numOfFrame; i++)
        frame_is_free[i] = 1;           // 1 for free, 0 for occupied
    /** TLB initialization */
    tlb = malloc(sizeof(entry) * TLB_SIZE);
    for(int i = 0; i < TLB_SIZE; i++){
        tlb[i].pageNum = -1;
        tlb[i].frameNum = -1;
        tlb[i].lruAge = MAX_LRU;
    }
    tlbHit = 0;
    /** Access number initialization */
    accessNum = 0;
}

int TLB_search(int pageNum)
{
    for(int i = 0; i < TLB_SIZE; i++){
        if(tlb[i].pageNum == pageNum){
            return tlb[i].frameNum;
        }
    }
    return NOT_FOUND;
}

void test()
{
    int address, offset;
    int pageNum, frameNum;
    signed char data;
    signed char *buffer;
    FILE *fp = fopen("./addresses.txt", "r");
    FILE *fp_correct = fopen("./correct.txt", "r");
    buffer = malloc(sizeof(char) * MAX_LENGTH);
    while(!feof(fp)){
        fscanf(fp, "%d", &address);
        pageNum = (address / pageSize) % numOfPage;
        offset = address % pageSize;
        frameNum = pageTable[pageNum];
        data = physicalMemory[frameNum][offset];
        printf("Virtual address: %d Physical address: %d Value: %d\n", address, frameNum * frameSize + offset, data);
    }
}

int main(int argc, char *argv[])
{
    int logicalAddress, pageNum, frameNum, offset;
    signed char data;
    signed char* buffer;
    /** File operation */
    FILE *fp = fopen(argv[1], "r");
    FILE *fp_disk = fopen("./BACKING_STORE.bin", "rb");
    assert(fp && fp_disk);      // In case of failure on file open

    /** Initialization */
    init();

    /** Operation */
    while(!feof(fp)){
        /** Update access number */
        accessNum++;
        fscanf(fp, "%d", &logicalAddress);
        /** Get page number and page offset */
        pageNum = (logicalAddress / pageSize) % numOfPage;
        offset = logicalAddress % pageSize;
        /** In TLB table? */
        if((frameNum = TLB_search(pageNum)) != NOT_FOUND){
            data = physicalMemory[frameNum][offset];
            tlbHit++;
            /** Update lruAge in TLB */
            for(int i = 0; i < TLB_SIZE; i++){
                if(tlb[i].pageNum == pageNum)
                    tlb[i].lruAge = MAX_LRU;
                else
                    tlb[i].lruAge--;
            }
            continue;
        }
        /** In page table? */
        if(pageTable[pageNum] != NOT_FOUND){
            frameNum = pageTable[pageNum];
            data = physicalMemory[frameNum][offset];
            /** Update TLB */
            int min_lru = MAX_LRU;
            int idx = 0;
            for(int i = 0; i < TLB_SIZE; i++) {
                if (tlb[i].lruAge < min_lru) {
                    min_lru = tlb[i].lruAge;
                    idx = i;
                }
            }
            /** Replace */
            tlb[idx].lruAge = MAX_LRU;
            tlb[idx].frameNum = frameNum;
            tlb[idx].pageNum = pageNum;
            for(int i = 0; i < TLB_SIZE; i++){
                if(tlb[i].pageNum == pageNum)
                    tlb[i].lruAge = MAX_LRU;
                else
                    tlb[i].lruAge--;
                }
            continue;
        }
        /** Page fault */
        /** Update page fault number */
        pageFault++;
        /** Read data from BACKING_STORE */
        buffer = malloc(sizeof(char) * pageSize);
        fseek(fp_disk, (pageNum) * pageSize, SEEK_SET);     // Move to the head of the page
        fread(buffer, sizeof(char), pageSize, fp_disk);     // Read the whole page into buffer
        /** Find a free page frame to store the data */
        int freeFrame_idx = -1;
        for(int i = 0; i < numOfFrame; i++) {
            if (frame_is_free[i] == 1) {
                freeFrame_idx = i;
                break;
            }
        }
        /** Memory is full, need page replacement */
        if(freeFrame_idx == -1){
            int min_lru = MAX_LRU;
            for(int i = 0; i < frameNum; i++){
                if(memLRU[i] < min_lru){
                    min_lru = memLRU[i];
                    freeFrame_idx = i;
                }
            }
            frameNum = freeFrame_idx;
            /** Update memory LRU */
            for(int i = 0; i < frameSize; i++){
                if(i == frameNum)
                    memLRU[i] = MAX_LRU;
                else
                    memLRU[i]--;
            }
        }
        else
            frameNum = freeFrame_idx;

        /** Memory copy */
        memcpy(physicalMemory[frameNum], buffer, pageSize);
        frame_is_free[frameNum] = 0;
        /** Update page table */
        pageTable[pageNum] = frameNum;
        /** Update TLB */
        int min_lru = MAX_LRU;
        int idx = 0;
        for(int i = 0; i < TLB_SIZE; i++){
            if(tlb[i].lruAge < min_lru){
                min_lru = tlb[i].lruAge;
                idx = i;
            }
        }
        tlb[idx].lruAge = MAX_LRU;
        tlb[idx].pageNum = pageNum;
        tlb[idx].frameNum = frameNum;
        for(int i = 0; i < TLB_SIZE; i++){
            if(tlb[i].pageNum != pageNum)
                tlb[i].lruAge--;
        }
    }
    accessNum--;

    test();

    printf("Successfully operate the simulation, the result is as follows:\n");
    printf("Page fault rate: %.3f\n", (double)pageFault / (double)accessNum);
    printf("TLB hit rate: %.3f\n", (double)tlbHit / (double)accessNum);

    return 0;
}
