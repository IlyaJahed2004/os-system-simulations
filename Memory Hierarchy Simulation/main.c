#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define CACHE_L1_SIZE 32
#define CACHE_L2_SIZE 256
#define MAIN_MEMORY_SIZE 1024
#define VIRTUAL_MEMORY_SIZE 4096

typedef struct {
    int data;
    int address;
    int valid;
    int access_time;
} MemoryBlock;

typedef struct {
    MemoryBlock l1_cache[CACHE_L1_SIZE];
    MemoryBlock l2_cache[CACHE_L2_SIZE];
    MemoryBlock main_memory[MAIN_MEMORY_SIZE];
    MemoryBlock virtual_memory[VIRTUAL_MEMORY_SIZE];

    int l1_hits;
    int l1_misses;
    int l2_hits;
    int l2_misses;
    int page_faults;
} MemoryHierarchy;

void initializeMemory(MemoryHierarchy *mh) {
    for (int i = 0; i < CACHE_L1_SIZE; i++) {
        mh->l1_cache[i].valid = 0;
        mh->l1_cache[i].access_time = 1;
        mh->l1_cache[i].address = -1;
    }

    for (int i = 0; i < CACHE_L2_SIZE; i++) {
        mh->l2_cache[i].valid = 0;
        mh->l2_cache[i].access_time = 10;
        mh->l2_cache[i].address = -1;
    }

    mh->l1_hits = mh->l1_misses = 0;
    mh->l2_hits = mh->l2_misses = 0;
    mh->page_faults = 0;
}

int accessMemory(MemoryHierarchy *mh, int address, int data) {
    int total_time = 0;

    int l1_idx = address % CACHE_L1_SIZE;
    total_time += mh->l1_cache[l1_idx].access_time;

    if (mh->l1_cache[l1_idx].valid &&
        mh->l1_cache[l1_idx].address == address) {

        mh->l1_hits++;
        mh->l1_cache[l1_idx].data = data;
        return total_time;
    }

    mh->l1_misses++;

    int l2_idx = address % CACHE_L2_SIZE;
    total_time += mh->l2_cache[l2_idx].access_time;

    if (mh->l2_cache[l2_idx].valid &&
        mh->l2_cache[l2_idx].address == address) {

        mh->l2_hits++;

        // Fill L1 from L2
        mh->l1_cache[l1_idx].valid = 1;
        mh->l1_cache[l1_idx].address = address;
        mh->l1_cache[l1_idx].data = data;

        return total_time;
    }

    mh->l2_misses++;

    // Miss in both: bring into L2 then L1 (simplified)
    mh->l2_cache[l2_idx].valid = 1;
    mh->l2_cache[l2_idx].address = address;
    mh->l2_cache[l2_idx].data = data;

    mh->l1_cache[l1_idx].valid = 1;
    mh->l1_cache[l1_idx].address = address;
    mh->l1_cache[l1_idx].data = data;

    return total_time;
}

void printMemoryStats(MemoryHierarchy *mh) {
    printf("\nPhase 2 Stats:\n");
    printf("L1 Hits: %d | Misses: %d\n", mh->l1_hits, mh->l1_misses);
    printf("L2 Hits: %d | Misses: %d\n", mh->l2_hits, mh->l2_misses);
}

int main() {
    MemoryHierarchy mh;
    srand(time(NULL));
    initializeMemory(&mh);

    int total_time = 0;
    int accesses = 1000;

    for (int i = 0; i < accesses; i++) {
        int address = rand() % 256;
        total_time += accessMemory(&mh, address, rand() % 1000);
    }

    printMemoryStats(&mh);
    printf("Average Access Time: %.2f cycles\n",
           (float)total_time / accesses);

    return 0;
}
