## Phase 1 – IPC Infrastructure and PU Ring Communication

In this phase, the core IPC mechanisms of the system are implemented.

### Features
- Shared memory segment for inter-process communication
- Dedicated mailbox (circular buffer) for each PU
- POSIX semaphores for synchronization (empty/full/mutex)
- Ring-based message passing between PUs

### Behavior
- The parent process generates prime numbers.
- Each prime is wrapped inside a `prime_t` structure.
- The initial target PU is selected using `prime % NUM_PUS`.
- Each PU:
  - Receives a message from its mailbox
  - Updates the value and counter
  - Forwards it to the next PU in a circular manner

This phase focuses purely on correct message passing and synchronization between processes.


## Phase 2 – Result Aggregation and Process Termination

This phase completes the system by adding result handling and clean shutdown logic.

### Added Features
- Shared result buffer for completed computations
- Semaphore-protected result queue
- Parent-side result collection and printing
- Graceful PU termination using a sentinel message

### Behavior
- When a PU decrements a `prime_t` counter to zero, the result is pushed into the shared result buffer.
- The parent process waits for and retrieves all results.
- After processing all primes, the parent sends a termination signal (`counter = -1`) to each PU.
- All child processes exit cleanly.

This phase ensures correctness, completeness, and proper lifecycle management of all processes.
