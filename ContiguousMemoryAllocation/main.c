#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#define MAX_MEMORY_SIZE 1024
#define MIN_PARTITION_SIZE 64


typedef struct MemoryBlock {
    size_t size;
    size_t start_address;
    bool is_allocated;
    struct MemoryBlock* next; 
} MemoryBlock;


//we can think of this as the Os's memory manager:
typedef struct {
    MemoryBlock* head;  //start of memory block list
    size_t total_size;
    size_t free_size;
    int allocation_strategy; // 1: First Fit (only)
} MemoryManager;

// Initialize memory manager and create a single free block covering all memory 
MemoryManager* initMemoryManager(size_t size, int strategy) {
    MemoryManager* manager = (MemoryManager*)malloc(sizeof(MemoryManager));
    if (!manager) return NULL;

    manager->total_size = size;
    manager->free_size = size;
    manager->allocation_strategy = strategy;

    MemoryBlock* block = (MemoryBlock*)malloc(sizeof(MemoryBlock));
    if (!block) { free(manager); return NULL; }

    block->size = size;
    block->start_address = 0;
    block->is_allocated = false;
    block->next = NULL;

    manager->head = block;
    return manager;
}

/* First Fit: return first free block that can satisfy the request */
MemoryBlock* firstFit(MemoryManager* manager, size_t size) {
    MemoryBlock* cur = manager->head;
    while (cur) {
        if (!cur->is_allocated && cur->size >= size)
            return cur;
        cur = cur->next;
    }
    return NULL;
}

/* Allocate memory using First Fit.
   in this first Phase we don't split the chosen block, it marks the whole block allocated. */
void* allocateMemory(MemoryManager* manager, size_t size) {
    if (!manager) return NULL;
    if (size < MIN_PARTITION_SIZE) return NULL;
    if (size > manager->free_size) return NULL;

    MemoryBlock* target = NULL;
    if (manager->allocation_strategy == 1) {
        target = firstFit(manager, size);
    } else {
        return NULL; // only First Fit implemented in Phase 1
    }

    if (!target) return NULL;

    // Phase1: allocate the entire block (no splitting)
    target->is_allocated = true;
    manager->free_size -= target->size;

    // Return start_address as a pointer (for demo purposes we use integer -> pointer)
    return (void*)(uintptr_t)target->start_address;
}

/* Free the block whose start address equals the given address */
void deallocate(MemoryManager* manager, void* address) {
    if (!manager) return;
    size_t addr = (size_t)(uintptr_t)address;
    MemoryBlock* cur = manager->head;
    while (cur) {
        if (cur->is_allocated && cur->start_address == addr) {
            cur->is_allocated = false;
            manager->free_size += cur->size;
            return;
        }
        cur = cur->next;
    }
    printf("Error: invalid address %zu\n", addr);
}

/* Print memory layout for debugging / logging */
void printMemory(MemoryManager* manager) {
    MemoryBlock* cur = manager->head;
    printf("[Memory Layout]: ");
    while (cur) {
        printf("[%s | Addr: %zu | Size: %zu] -> ",
               cur->is_allocated ? "ALLOC" : "FREE ",
               cur->start_address,
               cur->size);
        cur = cur->next;
    }
    printf("NULL\n");
}

/* Demo using only First Fit */
int main() {
    printf("Phase 1 demo: First Fit only\n");
    MemoryManager* mm = initMemoryManager(MAX_MEMORY_SIZE, 1);

    printf("Allocating A:200, B:200, C:200\n");
    void* A = allocateMemory(mm, 200);
    void* B = allocateMemory(mm, 200);
    void* C = allocateMemory(mm, 200);
    printMemory(mm);

    printf("\nFreeing B\n");
    deallocate(mm, B);
    printMemory(mm);

    printf("\nAllocating D:100 (should use first free that fits)\n");
    void* D = allocateMemory(mm, 100);
    printMemory(mm);

    free(mm); // note: blocks are not freed individually for this demo
    return 0;
}
