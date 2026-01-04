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