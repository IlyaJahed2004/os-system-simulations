# Log Analyzer – Phase 2

## Overview
Phase 2 completes the log analyzer by enforcing **dependency-aware execution order**.

While Phase 1 only respected dependencies during output generation, Phase 2 ensures that log files are processed in a correct logical order based on their declared dependencies.

The overall architecture remains the same, but execution order is improved.

---

## Key Difference from Phase 1

### Phase 1
- All log files are processed concurrently
- Dependencies affect only output order

### Phase 2
- Log files are sorted **before forking**
- Dependencies affect both execution and output
- A file is processed only after its dependency

---

## Dependency Enforcement
Before creating child processes:
- All discovered log files are sorted using dependency information
- If file A depends on file B, file B is processed first

This ensures:
- Correct logical processing order
- Deterministic execution behavior

---

## Execution Flow (Phase 2)

1. Discover all log files
2. Read dependency declarations
3. Sort files based on dependencies
4. Create pipes and fork workers in sorted order
5. Collect logs and bug reports
6. Generate final output

---

## Worker Logic
The worker logic remains unchanged:
- Reads a single file
- Skips dependency declaration line
- Validates logs using regex
- Sends matching logs and bug count to the parent

---

## Advantages of Phase 2
- Correct handling of dependent log files
- Cleaner and more predictable execution
- Fully satisfies assignment requirements

---

## Final Notes
- Phase 2 is the final phase of the assignment
- The implementation remains simple and readable
- No additional synchronization mechanisms are required