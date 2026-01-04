# Phase 1 — L1 Cache Only

## Goal
Implement L1 cache lookup and basic statistics (L1 hits, L1 misses) using the provided template.

## What this phase includes
- Direct-mapped L1 cache lookup (index = address % CACHE_L1_SIZE).
- Update L1 hit/miss counters.
- Account L1 access time for each access.
- Keep the rest of the memory hierarchy untouched for now.


# Phase 2 — Two-Level Cache Simulation (L1 + L2)

## Introduction
This phase extends the memory hierarchy simulator by introducing a second cache level (L2).
The goal is to model and analyze the behavior of a two-level cache hierarchy and understand
how L2 cache improves performance when L1 cache misses occur.

This phase builds directly on Phase 1 (L1-only cache) and preserves the same direct-mapped
cache structure while adding a new level of indirection and performance measurement.

---

## Memory Hierarchy Overview

CPU → L1 Cache → L2 Cache


### Cache Configuration
| Level | Size (blocks) | Mapping | Access Time |
|------|---------------|--------|-------------|
| L1   | 32            | Direct-mapped | 1 cycle |
| L2   | 256           | Direct-mapped | 10 cycles |

---

## Address Mapping
Both cache levels use a direct-mapped indexing scheme:

- **L1 index** = `address % CACHE_L1_SIZE`
- **L2 index** = `address % CACHE_L2_SIZE`

This simplified mapping allows a clear demonstration of:
- Conflict misses
- Cache promotion effects
- Performance differences between cache levels

---

## Memory Access Algorithm
For each memory access, the simulator follows the steps below:

1. **L1 Lookup**
   - If the block is present and valid → **L1 hit**
   - Access completes in **1 cycle**

2. **L1 Miss Handling**
   - L1 miss counter is incremented
   - L2 cache is probed

3. **L2 Lookup**
   - L2 access latency (**10 cycles**) is added
   - If the block is found → **L2 hit**

4. **L2 Hit Handling**
   - L2 hit counter is incremented
   - The block is promoted to L1 (write-allocate policy)
   - Total access time: **11 cycles**

5. **L2 Miss Handling**
   - L2 miss counter is incremented
   - The block is inserted into both L2 and L1
   - Lower memory levels are abstracted away in this phase

---

## Cache Update Policy
- **Write-Allocate**: On an L2 hit, the block is copied into L1.
- **Direct Replacement**: Because the caches are direct-mapped, each new block overwrites
  the existing block at the computed index.
- **Write Model**: Data is updated in the cache after each access (simplified write-through behavior).

---

## Performance Metrics
The simulator collects the following statistics:

- `L1 Hits`
- `L1 Misses`
- `L2 Hits`
- `L2 Misses`
- `Average Memory Access Time`

These metrics allow analysis of:
- Effectiveness of L1 cache
- Performance benefit introduced by the L2 cache
- Impact of locality on cache behavior

---

## Experimental Setup
- Number of memory accesses: **1000**
- Working set size: **256 addresses**
- Access pattern: Random accesses with temporal locality

This setup is designed to:
- Generate a mix of L1 hits, L2 hits, and misses
- Demonstrate the performance role of L2 cache

---

## Sample Observations
Typical outcomes from this phase include:
- A noticeable reduction in average access time compared to Phase 1
- High L2 hit rate after initial cold misses
- Improved overall cache efficiency due to block promotion from L2 to L1

---

## Limitations
This phase intentionally simplifies certain aspects of real systems:
- No main memory or virtual memory simulation
- No cache replacement policies beyond direct mapping
- No dirty bits or write-back mechanism

These aspects are introduced in subsequent phases.

---




