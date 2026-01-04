# Memory Hierarchy Simulator (L1, L2, Main Memory, Virtual Memory)

## Overview
This project implements a simplified but realistic **memory hierarchy simulator** in C, modeling
the behavior and performance of a modern computer memory system.

The simulator demonstrates how memory accesses propagate through multiple levels of storage,
including **L1 cache, L2 cache, main memory (RAM), and virtual memory**, while collecting
performance statistics such as cache hits, misses, page faults, and average access time.

The goal of this project is educational: to clearly illustrate **locality, cache efficiency,
miss penalties, and page fault behavior** using a clean and understandable implementation.

---

## Memory Hierarchy Model

CPU

↓

L1 Cache (32 blocks, 1 cycle)

↓

L2 Cache (256 blocks, 10 cycles)

↓

Main Memory (1024 blocks, 100 cycles)

↓

Virtual Memory (4096 blocks, 1000 cycles)


---

## Cache and Memory Configuration

| Level            | Size (Blocks) | Mapping         | Access Time |
|------------------|---------------|-----------------|-------------|
| L1 Cache         | 32            | Direct-mapped   | 1 cycle     |
| L2 Cache         | 256           | Direct-mapped   | 10 cycles   |
| Main Memory      | 1024          | Direct-mapped   | 100 cycles  |
| Virtual Memory   | 4096          | Direct-mapped   | 1000 cycles |

---

## Address Mapping
All cache and memory levels use a **direct-mapped indexing scheme**:

- **Index = address % size**

The full address is stored as a simplified tag to detect hits and misses.
This abstraction keeps the implementation readable while preserving realistic behavior.

---

## Memory Access Algorithm

For each memory access, the simulator performs the following steps:

1. **L1 Cache Lookup**
   - If the block is valid and the address matches → **L1 hit**
   - Access completes in 1 cycle

2. **L1 Miss → L2 Cache Lookup**
   - L2 access latency (10 cycles) is added
   - On a hit, the block is promoted to L1 (write-allocate)

3. **L2 Miss → Main Memory Lookup**
   - Main memory access latency (100 cycles) is added
   - If the block is resident, it is loaded into L2 and L1

4. **Main Memory Miss → Page Fault**
   - Data is fetched from virtual memory (1000 cycles)
   - The page is installed into main memory
   - The block is then promoted to L2 and L1

This process models the **increasing miss penalty** across deeper levels of the hierarchy.

---

## Cache Policies

- **Mapping:** Direct-mapped at all levels
- **Write Policy:** Write-allocate with simplified write-through behavior
- **Replacement Policy:** Direct replacement (implicit due to direct mapping)
- **Promotion:** On a hit at a lower level, blocks are promoted upward

---

## Experimental Setup

- **Number of memory accesses:** 1000
- **Working set size:** 256 addresses
- **Access pattern:**
  - 90% of accesses target the working set (high locality)
  - 10% of accesses are random across virtual memory

This setup highlights:
- Cache warm-up behavior
- The performance benefit of L2 cache
- The cost of page faults

---

## Collected Metrics

The simulator reports:

- L1 cache hits and misses
- L2 cache hits and misses
- Number of page faults
- Total memory access time
- Average memory access time (AMAT)

These metrics allow quantitative analysis of cache effectiveness and memory latency.

---

## Project Phases

This project was developed incrementally:

1. **Phase 1:** L1 cache simulation and hit/miss tracking
2. **Phase 2:** Two-level cache hierarchy (L1 + L2)
3. **Final Phase:** Full memory hierarchy including main memory and virtual memory with page faults

Each phase builds upon the previous one to gradually introduce more realistic system behavior.

---

