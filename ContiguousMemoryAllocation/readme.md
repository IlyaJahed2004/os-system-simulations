# Contiguous Memory Allocator — Phase 1 (First Fit)

This is Phase 1 of a small contiguous memory allocation simulator in C.
Phase 1 implements a minimal allocator using the First Fit strategy.

Implemented features:
- Memory manager initialization (single free block)
- First Fit allocation (no splitting; whole block is allocated)
- Deallocation by start address
- Simple memory layout logging for each operation

Limitations (intended for Phase 1):
- Allocation does not split a larger free block into allocated + leftover free block
- No compaction or merging of free blocks
- The returned address is a simulated integer address (for demonstration)


This phase is designed as a clean first commit and will be extended in following phases.


# Contiguous Memory Allocator — Phase 2 (First/Best/Worst Fit + Splitting)

Phase 2 extends the simulator by adding Best Fit and Worst Fit strategies and supports splitting a free block when allocating.
Splitting occurs when the chosen block is larger than the request plus a threshold (`MIN_PARTITION_SIZE`).

Implemented features:
- First Fit, Best Fit, Worst Fit selection
- Block splitting on allocation (leftover becomes a free block if large enough)
- Deallocation (marks block free)
- Logging of memory layout to observe fragmentation behavior


# Contiguous Memory Allocator — Phase 3 (Compaction)

Phase 3 finalizes the simulator by adding memory compaction: allocated blocks are relocated to the beginning of memory and free space is merged into a single block.

Features:
- First Fit, Best Fit, Worst Fit strategies
- Block splitting on allocation
- Deallocation
- Memory compaction (relocate allocated blocks + merge remaining free space)
- Logging to observe fragmentation and the effect of compaction

