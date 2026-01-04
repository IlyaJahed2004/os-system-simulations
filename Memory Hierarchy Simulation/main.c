// 3.c — Phase 1: L1 Cache Only
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
        mh->l1_cache[i].access_time = 1; // L1 = 1 cycle
        mh->l1_cache[i].address = -1;
    }

    mh->l1_hits = 0;
    mh->l1_misses = 0;
    mh->l2_hits = 0;
    mh->l2_misses = 0;
    mh->page_faults = 0;
}

int accessMemory(MemoryHierarchy *mh, int address, int data) {
    int total_time = 0;

    // Direct-mapped L1 index
    int l1_idx = address % CACHE_L1_SIZE;

    // Cost of checking L1
    total_time += mh->l1_cache[l1_idx].access_time;

    // L1 hit
    if (mh->l1_cache[l1_idx].valid &&
        mh->l1_cache[l1_idx].address == address) {

        mh->l1_hits++;
        mh->l1_cache[l1_idx].data = data; // simulate write
        return total_time;
    }

    // L1 miss
    mh->l1_misses++;

    // Bring block into L1 (no lower levels yet)
    mh->l1_cache[l1_idx].valid = 1;
    mh->l1_cache[l1_idx].address = address;
    mh->l1_cache[l1_idx].data = data;

    return total_time;
}

void printMemoryStats(MemoryHierarchy *mh) {
    printf("\nPhase 1 Stats:\n");
    printf("L1 Hits: %d\n", mh->l1_hits);
    printf("L1 Misses: %d\n", mh->l1_misses);
}

int main() {
    MemoryHierarchy mh;
    srand(time(NULL));
    initializeMemory(&mh);

    int total_accesses = 1000;
    int total_time = 0;

    for (int i = 0; i < total_accesses; i++) {
        int address = rand() % 256; // small working set
        int data = rand() % 1000;
        total_time += accessMemory(&mh, address, data);
    }

    printMemoryStats(&mh);
    printf("Average Access Time: %.2f cycles\n",
           (float)total_time / total_accesses);

    return 0;
}
