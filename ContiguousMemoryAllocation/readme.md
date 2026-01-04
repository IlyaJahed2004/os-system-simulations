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
