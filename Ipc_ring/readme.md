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
