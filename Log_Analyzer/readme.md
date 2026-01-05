# Log Analyzer – Phase 1

## Overview
This project is a simple log analyzer implemented in C as part of a System Programming assignment.

Phase 1 focuses on the core system programming concepts:
- Parsing structured log files using Regular Expressions
- Concurrent processing using `fork()`
- Inter-process communication using `pipe()`
- Basic handling of file dependencies
- Detecting malformed log entries (bugs)

The implementation prioritizes simplicity, readability, and correctness over advanced optimizations.

---

## Log File Format
Each log file contains text lines. A valid log line must strictly follow this format:

LEVEL | YYYY-MM-DD HH:MM:SS | Message

makefile
Copy code

Example:
ERROR | 2024-01-15 10:42:01 | Disk read failure

yaml
Copy code

Supported severity levels:
- `ERROR`
- `INFO`
- `WARNING`

---

## Dependency Declaration
A log file may declare a dependency in its **first line** using the following syntax:

... other_file.txt

yaml
Copy code

This means that the current log file depends on `other_file.txt`.

In **Phase 1**, dependencies are:
- Detected and stored
- Used only to control **output order**
- **Not enforced during execution**

---

## Program Architecture

### Global Data
- All discovered log files are stored in a global `files[]` array
- Each entry stores:
  - File name
  - Full path
  - Dependency (if any)
  - Pipe file descriptors
  - Bug count

---

## Execution Flow

### 1. Initialization
- Configuration values are set (target severity, logs directory, output file)
- A regular expression is compiled to validate log format

---

### 2. File Discovery
- The program scans the `logs/` directory
- All `.txt` files are discovered
- For each file:
  - Filename and full path are stored
  - The first line is checked for a dependency declaration

---

### 3. Concurrent Processing
For each discovered file:
- A pipe is created
- A child process is created using `fork()`

Each **child process**:
- Opens its assigned log file
- Skips the dependency declaration line (if present)
- Validates each log line using regex
- Sends matching logs to the parent using:
LOG:<log line>

cpp
Copy code
- Counts malformed log lines
- Sends the final bug count using:
BUG:<count>

yaml
Copy code

The **parent process**:
- Closes unused pipe ends
- Waits for all child processes to finish

---

### 4. Dependency-Based Output Ordering
After all workers finish:
- Log files are sorted using a simple bubble sort
- If file A depends on file B, file B appears first in output

---

### 5. Output Generation
- All `LOG:` messages are written to `output.txt`
- A bug summary is printed at the bottom of the file in the format:
filename.txt: X bugs

yaml
Copy code

---

## Output Example

ERROR | 2024-01-15 10:42:01 | Disk read failure
ERROR | 2024-01-15 11:03:19 | Network timeout

log1.txt: 2 bugs
log2.txt: 0 bugs

pgsql
Copy code

---

## Limitations of Phase 1
- Dependencies are not enforced during execution
- All files are processed concurrently
- Error handling is minimal by design

These limitations are addressed in Phase 2.