# Memory Hierarchy Simulator (L1, L2, Main Memory, Virtual Memory) - LRU Version

## Overview
This project implements a **memory hierarchy simulator** in C, now enhanced with **LRU (Least Recently Used) replacement policy** for both L1 and L2 caches, as well as main memory. The simulator models CPU memory accesses and shows how data propagates through **L1 cache, L2 cache, main memory, and virtual memory**, while collecting performance statistics such as hits, misses, page faults, total access time, and average memory access time (AMAT).

The purpose is educational: to illustrate the effects of **locality, cache replacement policies, miss penalties, and page fault handling** in a realistic, yet understandable way.

---

## Memory Hierarchy Model

CPU

↓  

L1 Cache (32 blocks, 1 cycle, LRU)

↓  

L2 Cache (256 blocks, 10 cycles, LRU)

↓  

Main Memory (1024 blocks, 100 cycles, LRU)

↓  

Virtual Memory (4096 blocks, 1000 cycles)

---

## Cache and Memory Configuration

| Level            | Size (Blocks) | Replacement Policy | Access Time |
|------------------|---------------|------------------|-------------|
| L1 Cache         | 32            | LRU              | 1 cycle     |
| L2 Cache         | 256           | LRU              | 10 cycles   |
| Main Memory      | 1024          | LRU              | 100 cycles  |
| Virtual Memory   | 4096          | Not applicable   | 1000 cycles |

---

## Address Mapping
All caches and memory levels use a **direct-mapped indexing scheme for simplicity**. The full address is stored as a tag for hit/miss detection.  

- **Index = address % size**  

This allows fast lookup while still simulating realistic memory behavior.

---

## Memory Access Algorithm (LRU-based)

For each memory access:

1. **L1 Cache Lookup**
   - Add L1 access latency
   - If block valid and address matches → **L1 hit**
   - Update `last_access_time` for LRU

2. **L1 Miss → L2 Cache Lookup**
   - Add L2 access latency
   - On L2 hit, promote block to L1 (using LRU victim if needed)
   - Update `last_access_time` for LRU

3. **L2 Miss → Main Memory Lookup**
   - Add main memory latency
   - If block resident, load into L2 then L1 (using LRU slots)
   - Update `last_access_time` accordingly

4. **Main Memory Miss → Page Fault**
   - Fetch from virtual memory (1000 cycles)
   - Install into main memory using LRU victim
   - Promote block into L2 and L1

This simulates the **gradually increasing penalty** for cache misses and page faults, with LRU replacement ensuring that frequently used blocks stay in faster memory levels.

---

## Cache Policies

- **Mapping:** Direct-mapped for simplicity
- **Replacement:** LRU for L1, L2, and main memory
- **Write Policy:** Write-allocate with simplified write-through
- **Promotion:** Blocks hit at lower levels are moved upward to faster memory

---

## Experimental Setup

- **Memory accesses:** 1000
- **Working set size:** 256 addresses
- **Access pattern:**  
  - 90% of accesses target the working set (high locality)  
  - 10% of accesses are random across virtual memory  

This setup emphasizes cache warm-up, the benefit of L2, and page fault costs.

---

## Collected Metrics

- L1 cache hits and misses
- L2 cache hits and misses
- Number of page faults
- Total access time
- Average memory access time (AMAT)

These metrics allow quantitative evaluation of LRU effectiveness.

---

## Project Phases

1. **Phase 1:** L1 cache simulation with direct-mapped replacement and hit/miss statistics  
2. **Phase 2:** Two-level cache hierarchy (L1 + L2), still direct-mapped  
3. **Phase 3:** Full memory hierarchy including main memory and virtual memory with page faults  
4. **Current LRU Version:** Introduced **LRU replacement** for L1, L2, and main memory to make replacement smarter and more realistic

---
