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

typedef struct {
    MemoryBlock* head;
    size_t total_size;
    size_t free_size;
    int allocation_strategy; // 1: First Fit, 2: Best Fit, 3: Worst Fit
} MemoryManager;

/* Initialize memory manager with a single free block */
MemoryManager* initMemoryManager(size_t size, int strategy) {
    MemoryManager* manager = (MemoryManager*)malloc(sizeof(MemoryManager));
    if (!manager) return NULL;
    manager->total_size = size;
    manager->free_size = size;
    manager->allocation_strategy = strategy;
    MemoryBlock* b = (MemoryBlock*)malloc(sizeof(MemoryBlock));
    if (!b) { free(manager); return NULL; }
    b->size = size;
    b->start_address = 0;
    b->is_allocated = false;
    b->next = NULL;
    manager->head = b;
    return manager;
}

/* First Fit: return first free block that fits */
MemoryBlock* firstFit(MemoryManager* manager, size_t size) {
    MemoryBlock* cur = manager->head;
    while (cur) {
        if (!cur->is_allocated && cur->size >= size) return cur;
        cur = cur->next;
    }
    return NULL;
}

/* Best Fit: return smallest free block that fits */
MemoryBlock* bestFit(MemoryManager* manager, size_t size) {
    MemoryBlock* cur = manager->head;
    MemoryBlock* best = NULL;
    while (cur) {
        if (!cur->is_allocated && cur->size >= size) {
            if (!best || cur->size < best->size) best = cur;
        }
        cur = cur->next;
    }
    return best;
}

/* Worst Fit: return largest free block that fits */
MemoryBlock* worstFit(MemoryManager* manager, size_t size) {
    MemoryBlock* cur = manager->head;
    MemoryBlock* worst = NULL;
    while (cur) {
        if (!cur->is_allocated && cur->size >= size) {
            if (!worst || cur->size > worst->size) worst = cur;
        }
        cur = cur->next;
    }
    return worst;
}

/* Allocate memory based on selected strategy.
   If chosen block is much larger than requested and leftover >= MIN_PARTITION_SIZE,
   split the block into allocated + free leftover.
*/
void* allocateMemory(MemoryManager* manager, size_t size)
{
    if (!manager) return NULL;
    if (size < MIN_PARTITION_SIZE) return NULL;
    if (size > manager->free_size) return NULL;

    MemoryBlock* target = NULL;
    switch (manager->allocation_strategy) {
        case 1: target = firstFit(manager, size); break;
        case 2: target = bestFit(manager, size); break;
        case 3: target = worstFit(manager, size); break;
        default: return NULL;
    }
    if (!target) return NULL;

    // If the block is much larger, split it
    if (target->size > size + MIN_PARTITION_SIZE) {
        MemoryBlock* leftover = (MemoryBlock*)malloc(sizeof(MemoryBlock));
        leftover->size = target->size - size;
        leftover->start_address = target->start_address + size;
        leftover->is_allocated = false;
        leftover->next = target->next;

        target->size = size;
        target->next = leftover;
    }

    target->is_allocated = true;
    manager->free_size -= target->size;
    return (void*)(uintptr_t)target->start_address;
}

/* Deallocate: mark block free and increase free_size.
   Note: merging of adjacent free blocks will be handled by compaction in Phase 3. */
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

/* Print current memory layout */
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

/* Demo: run the same scenario for each strategy */
int main() {
    const char* names[] = {"First Fit", "Best Fit", "Worst Fit"};
    for (int s = 1; s <= 3; ++s) {
        printf("\n=== Strategy %d: %s ===\n", s, names[s-1]);
        MemoryManager* mm = initMemoryManager(MAX_MEMORY_SIZE, s);

        printf("Allocating A:200, B:200, C:200\n");
        void* A = allocateMemory(mm, 200);
        void* B = allocateMemory(mm, 200);
        void* C = allocateMemory(mm, 200);
        printMemory(mm);

        printf("\nFreeing B\n");
        deallocate(mm, B);
        printMemory(mm);

        printf("\nAllocating D:100\n");
        allocateMemory(mm, 100);
        printMemory(mm);

        free(mm); // demo cleanup
    }
    return 0;
}
