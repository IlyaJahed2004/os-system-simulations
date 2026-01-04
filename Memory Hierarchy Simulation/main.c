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
    unsigned long last_access_time; 
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

    unsigned long counter; 
} MemoryHierarchy;

// Initialize memory hierarchy
void initializeMemory(MemoryHierarchy *mh) {
    for (int i = 0; i < CACHE_L1_SIZE; i++) {
        mh->l1_cache[i].valid = 0;
        mh->l1_cache[i].access_time = 1;
        mh->l1_cache[i].last_access_time = 0;
        mh->l1_cache[i].address = -1;
        mh->l1_cache[i].data = 0;
    }

    for (int i = 0; i < CACHE_L2_SIZE; i++) {
        mh->l2_cache[i].valid = 0;
        mh->l2_cache[i].access_time = 10;
        mh->l2_cache[i].last_access_time = 0;
        mh->l2_cache[i].address = -1;
        mh->l2_cache[i].data = 0;
    }

    for (int i = 0; i < MAIN_MEMORY_SIZE; i++) {
        mh->main_memory[i].data = rand() % 1000;
        mh->main_memory[i].address = i;
        mh->main_memory[i].valid = 1; 
        mh->main_memory[i].access_time = 100;
        mh->main_memory[i].last_access_time = 0;
    }

    for (int i = 0; i < VIRTUAL_MEMORY_SIZE; i++) {
        mh->virtual_memory[i].data = rand() % 1000;
        mh->virtual_memory[i].address = i;
        mh->virtual_memory[i].valid = 1;
        mh->virtual_memory[i].access_time = 1000;
        mh->virtual_memory[i].last_access_time = 0;
    }

    mh->l1_hits = mh->l1_misses = mh->l2_hits = mh->l2_misses = mh->page_faults = 0;
    mh->counter = 0;
}

int getLRUVictim(MemoryBlock cache[], int size) {
    for (int i = 0; i < size; i++) {
        if (!cache[i].valid) return i;
    }
    // here if all the slots are valid we should pick smallest last_access_time
    int victim = 0;
    unsigned long min_time = cache[0].last_access_time;
    for (int i = 1; i < size; i++) {
        if (cache[i].last_access_time < min_time) {
            min_time = cache[i].last_access_time;
            victim = i;
        }
    }
    return victim;
}

// Memory access function which is LRU-based in this version
int accessMemory(MemoryHierarchy *mh, int address, int data) {
    int total_time = 0;
    mh->counter++;

    //  L1 probe
    total_time += 1; 
    int l1_hit = -1;
    for (int i = 0; i < CACHE_L1_SIZE; i++) {
        if (mh->l1_cache[i].valid && mh->l1_cache[i].address == address) {
            l1_hit = i;
            break;
        }
    }

    if (l1_hit != -1) {
        mh->l1_hits++;
        mh->l1_cache[l1_hit].last_access_time = mh->counter;
        mh->l1_cache[l1_hit].data = data; 
        return total_time;
    }
    mh->l1_misses++;

    //L2 probe 
    total_time += 10; 
    int l2_hit = -1;
    for (int i = 0; i < CACHE_L2_SIZE; i++) {
        if (mh->l2_cache[i].valid && mh->l2_cache[i].address == address) {
            l2_hit = i;
            break;
        }
    }

    
    int l1_target = getLRUVictim(mh->l1_cache, CACHE_L1_SIZE);

    if (l2_hit != -1) {
        mh->l2_hits++;
        mh->l2_cache[l2_hit].last_access_time = mh->counter;

        // bring into L1
        mh->l1_cache[l1_target] = mh->l2_cache[l2_hit];
        mh->l1_cache[l1_target].last_access_time = mh->counter;
        mh->l1_cache[l1_target].access_time = 1;
        mh->l1_cache[l1_target].valid = 1;
        mh->l1_cache[l1_target].data = data; 
        return total_time;
    }
    mh->l2_misses++;

    //  Main memory probe 
    total_time += 100;
    int mm_index = -1;
    for (int i = 0; i < MAIN_MEMORY_SIZE; i++) {
        if (mh->main_memory[i].valid && mh->main_memory[i].address == address) {
            mm_index = i;
            break;
        }
    }

    int l2_target = getLRUVictim(mh->l2_cache, CACHE_L2_SIZE);

    if (mm_index != -1) {
        mh->main_memory[mm_index].last_access_time = mh->counter;

        // bring to L2
        mh->l2_cache[l2_target] = mh->main_memory[mm_index];
        mh->l2_cache[l2_target].last_access_time = mh->counter;
        mh->l2_cache[l2_target].access_time = 10;
        mh->l2_cache[l2_target].valid = 1;

        // bring to L1
        mh->l1_cache[l1_target] = mh->main_memory[mm_index];
        mh->l1_cache[l1_target].last_access_time = mh->counter;
        mh->l1_cache[l1_target].access_time = 1;
        mh->l1_cache[l1_target].valid = 1;
        mh->l1_cache[l1_target].data = data; 
        return total_time;
    }

    //  Page fault
    mh->page_faults++;
    int vm_index = address % VIRTUAL_MEMORY_SIZE;
    total_time += mh->virtual_memory[vm_index].access_time;

    int mm_target = getLRUVictim(mh->main_memory, MAIN_MEMORY_SIZE);

    // swapping the page into main memory
    mh->main_memory[mm_target] = mh->virtual_memory[vm_index];
    mh->main_memory[mm_target].address = address;
    mh->main_memory[mm_target].last_access_time = mh->counter;
    mh->main_memory[mm_target].access_time = 100;
    mh->main_memory[mm_target].valid = 1;

    // now bring into L2 and L1
    mh->l2_cache[l2_target] = mh->main_memory[mm_target];
    mh->l2_cache[l2_target].last_access_time = mh->counter;
    mh->l2_cache[l2_target].access_time = 10;
    mh->l2_cache[l2_target].valid = 1;

    mh->l1_cache[l1_target] = mh->main_memory[mm_target];
    mh->l1_cache[l1_target].last_access_time = mh->counter;
    mh->l1_cache[l1_target].access_time = 1;
    mh->l1_cache[l1_target].valid = 1;
    mh->l1_cache[l1_target].data = data; 

    return total_time;
}

void printMemoryStats(MemoryHierarchy *mh) {
    printf("\nMemory Access Statistics:\n");
    printf("L1 Cache Hits: %d\n", mh->l1_hits);
    printf("L1 Cache Misses: %d\n", mh->l1_misses);
    printf("L2 Cache Hits: %d\n", mh->l2_hits);
    printf("L2 Cache Misses: %d\n", mh->l2_misses);
    printf("Page Faults: %d\n", mh->page_faults);

    int l1_total = mh->l1_hits + mh->l1_misses;
    int l2_total = mh->l2_hits + mh->l2_misses;

    float l1_hit_ratio = (l1_total == 0) ? 0.0f : (float)mh->l1_hits / l1_total;
    float l2_hit_ratio = (l2_total == 0) ? 0.0f : (float)mh->l2_hits / l2_total;

    printf("\nCache Performance:\n");
    printf("L1 Hit Ratio: %.2f%%\n", l1_hit_ratio * 100.0f);
    printf("L2 Hit Ratio: %.2f%%\n", l2_hit_ratio * 100.0f);
}

int main() {
    MemoryHierarchy mh;
    srand(time(NULL));
    initializeMemory(&mh);

    int total_accesses = 1000;
    int total_time = 0;
    int working_set = 256;

    for (int i = 0; i < total_accesses; i++) {
        int address;
        if (rand() % 10 < 9) {
            address = rand() % working_set;
        } else {
            address = rand() % VIRTUAL_MEMORY_SIZE;
        }
        int data = rand() % 1000;
        total_time += accessMemory(&mh, address, data);
    }

    printMemoryStats(&mh);
    printf("\nTotal Access Time: %d cycles\n", total_time);
    printf("Average Access Time: %.2f cycles\n", (float)total_time / total_accesses);

    return 0;
}
