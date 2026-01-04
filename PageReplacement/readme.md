# Page Replacement Algorithms in C

## Stage 1: FIFO Page Replacement Algorithm

This project demonstrates classic **Operating System page replacement algorithms** implemented in **C**.  
In this first stage, the **FIFO (First-In First-Out)** algorithm is implemented and tested.

---

##  FIFO Algorithm Overview

FIFO replaces the **oldest page in memory** when a page fault occurs and no free frame is available.

### Key Characteristics:
- Simple to implement
- Uses a circular pointer to track replacement order
- Does **not** consider page usage frequency or recency

---

##  Implementation Details

- Memory frames are initialized to `-1` (empty)
- A circular pointer (`stack_pointer`) simulates queue behavior
- On page fault:
  - If a free frame exists → insert page
  - Otherwise → replace the oldest page

Each page access prints:
- Whether it caused a **page fault**
- The current state of memory frames

---

## Stage 2: FIFO + LRU Page Replacement

This stage extends the project by adding the **LRU (Least Recently Used)** page replacement algorithm alongside FIFO.

---

##  LRU Algorithm Overview

LRU replaces the page that has **not been used for the longest time**.

### Key Characteristics:
- More efficient than FIFO
- Tracks access history
- Approximates real-world memory behavior

---

##  Implementation Details

Each memory frame stores:
- `page_num` → page number
- `last_used` → timestamp of last access

A global `timeCounter` is incremented on every page access.

### On Page Fault:
1. If an empty frame exists → insert page
2. Else → replace the page with the **smallest last_used value**

---

## Stage 3: FIFO, LRU, and Optimal Page Replacement

This final stage completes the project by implementing the **Optimal Page Replacement** algorithm.

---

##  Optimal Algorithm Overview

The Optimal algorithm replaces the page that will be **used farthest in the future**.

### Key Characteristics:
- Produces the **minimum possible page faults**
- Requires future knowledge (not practical in real systems)
- Used as a theoretical benchmark

---

##  Implementation Logic

For each page fault:
1. Check when each page in memory will be used next
2. If a page is **never used again**, replace it immediately
3. Otherwise, replace the page with the **farthest future use**

---
