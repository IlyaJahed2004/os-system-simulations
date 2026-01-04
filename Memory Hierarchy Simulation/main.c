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

// Initialize memory hierarchy
void initializeMemory(MemoryHierarchy *mh) {
    // Initialize L1 Cache
    for(int i = 0; i < CACHE_L1_SIZE; i++) {
        mh->l1_cache[i].valid = 0;
        mh->l1_cache[i].access_time = 1; // 1 cycle
        mh->l1_cache[i].data = 0;
        mh->l1_cache[i].address = -1;
    }
    
    // Initialize L2 Cache
    for(int i = 0; i < CACHE_L2_SIZE; i++) {
        mh->l2_cache[i].valid = 0;
        mh->l2_cache[i].access_time = 10; // 10 cycles
        mh->l2_cache[i].data = 0;
        mh->l2_cache[i].address = -1;
    }
    
    // Initialize Main Memory with a resident working set
    // We treat main memory slots as "resident" for addresses 0..MAIN_MEMORY_SIZE-1 initially.
    for(int i = 0; i < MAIN_MEMORY_SIZE; i++) {
        mh->main_memory[i].data = rand() % 1000;
        mh->main_memory[i].address = i; // first 1K virtual pages are resident
        mh->main_memory[i].valid = 1;
        mh->main_memory[i].access_time = 100; // 100 cycles
    }
    
    // Initialize Virtual Memory with backing data
    for(int i = 0; i < VIRTUAL_MEMORY_SIZE; i++) {
        mh->virtual_memory[i].data = rand() % 1000;
        mh->virtual_memory[i].address = i;
        mh->virtual_memory[i].valid = 1;
        mh->virtual_memory[i].access_time = 1000; // 1000 cycles
    }
    
    mh->l1_hits = 0;
    mh->l1_misses = 0;
    mh->l2_hits = 0;
    mh->l2_misses = 0;
    mh->page_faults = 0;
}

// Memory access function
int accessMemory(MemoryHierarchy *mh, int address, int data) {
    int total_time = 0;

    // --- Full hierarchy logic (L1 -> L2 -> Main -> Virtual) ---
    // We model direct-mapped caches and direct-mapped main memory slots.
    // Each probe pays that level's access_time even if it's a miss (simulating tag check cost).

    // L1 probe
    int l1_idx = address % CACHE_L1_SIZE;
    total_time += mh->l1_cache[l1_idx].access_time;

    // If valid and tag matches -> L1 hit
    if (mh->l1_cache[l1_idx].valid && mh->l1_cache[l1_idx].address == address) {
        mh->l1_hits++;
        // Simulate write-allocate/write-through: update stored block
        mh->l1_cache[l1_idx].data = data;
        return total_time;
    }

    // L1 miss
    mh->l1_misses++;

    // L2 probe
    int l2_idx = address % CACHE_L2_SIZE;
    total_time += mh->l2_cache[l2_idx].access_time;

    if (mh->l2_cache[l2_idx].valid && mh->l2_cache[l2_idx].address == address) {
        // L2 hit: bring into L1 (direct-mapped replacement)
        mh->l2_hits++;

        mh->l1_cache[l1_idx].valid = 1;
        mh->l1_cache[l1_idx].address = address;
        mh->l1_cache[l1_idx].data = mh->l2_cache[l2_idx].data;

        // Update with requested data (simulate a write)
        mh->l1_cache[l1_idx].data = data;
        return total_time;
    }

    // L2 miss
    mh->l2_misses++;

    // Main memory probe (check if resident)
    int mm_idx = address % MAIN_MEMORY_SIZE;
    total_time += mh->main_memory[mm_idx].access_time;

    if (mh->main_memory[mm_idx].valid && mh->main_memory[mm_idx].address == address) {
        // Found in main memory: fill L2 then L1
        mh->l2_cache[l2_idx].valid = 1;
        mh->l2_cache[l2_idx].address = address;
        mh->l2_cache[l2_idx].data = mh->main_memory[mm_idx].data;

        mh->l1_cache[l1_idx].valid = 1;
        mh->l1_cache[l1_idx].address = address;
        mh->l1_cache[l1_idx].data = mh->l2_cache[l2_idx].data;

        // simulate write
        mh->l1_cache[l1_idx].data = data;
        return total_time;
    } else {
        // Page fault: bring from virtual backing store
        mh->page_faults++;

        int vm_idx = address % VIRTUAL_MEMORY_SIZE;
        // cost to read from virtual backing store (slow)
        total_time += mh->virtual_memory[vm_idx].access_time;

        // Install into main memory slot (overwrite simple model)
        mh->main_memory[mm_idx].valid = 1;
        mh->main_memory[mm_idx].address = address;
        mh->main_memory[mm_idx].data = mh->virtual_memory[vm_idx].data;

        // cost to access main after swap-in
        total_time += mh->main_memory[mm_idx].access_time;

        // Fill L2 and L1 (write-allocate)
        mh->l2_cache[l2_idx].valid = 1;
        mh->l2_cache[l2_idx].address = address;
        mh->l2_cache[l2_idx].data = mh->main_memory[mm_idx].data;

        mh->l1_cache[l1_idx].valid = 1;
        mh->l1_cache[l1_idx].address = address;
        mh->l1_cache[l1_idx].data = mh->l2_cache[l2_idx].data;

        // simulate write with provided data
        mh->l1_cache[l1_idx].data = data;

        return total_time;
    }

    // unreachable
    return total_time;
}

// Print memory statistics
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
    int working_set = 256; // encourage locality
    
    for(int i = 0; i < total_accesses; i++) {
        // 90% of the time stay within a small working set to produce cache hits
        int address;
        if(rand() % 10 < 9) {
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
