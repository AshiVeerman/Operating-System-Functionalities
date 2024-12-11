# Reader-Writer Locks and Producer-Consumer Problem

This project implements the following classic concurrency problems in C:

1. **Single Producer-Single Consumer Problem**:
   - Simulates producer-consumer interaction using a circular buffer.
   - Utilizes mutex locks and condition variables to ensure thread synchronization.

2. **Reader-Writer Locks**:
   - Implements both reader-preference and writer-preference versions to manage access to a shared resource.

---

## Part 1: Single Producer-Single Consumer Problem

### Problem Description
Simulates a producer-consumer scenario with the following details:
- **Shared Resource**: A circular buffer of size 100, holding unsigned integer values.
- **Producer Thread**:
  - Reads integers continuously from an input file (`input-part1.txt`).
  - Adds them to the buffer.
  - Waits when the buffer is full.
- **Consumer Thread**:
  - Consumes items from the buffer.
  - Waits when the buffer is empty.
- **Synchronization**: Achieved using mutex locks and condition variables.

### Input
- **File**: `input-part1.txt` contains a list of unsigned integers separated by whitespace.

### Output
- Producer and consumer actions, such as producing and consuming items, are printed to the console.

---

## Part 2: Reader-Writer Locks

### Problem Description
Simulates a shared resource accessed by multiple reader and writer threads:
1. **Reader-Preference**:
   - Allows multiple readers to access the resource simultaneously.
   - Ensures that no writer can access the resource while readers are active.
2. **Writer-Preference**:
   - Gives priority to writers over readers.
   - Ensures that no readers can access the resource if a writer is waiting.

### Input
- **Command-line Arguments**: 
  - Number of reader threads `n`.
  - Number of writer threads `m`.

### Output
- Actions performed by readers and writers, including accessing or waiting for the resource, are printed to the console.

---

## Compilation and Execution

### **Requirements**
- GCC Compiler.
- POSIX-compliant system.

---

## Key Features
- Synchronization using mutex locks and condition variables.
- Implements both priority modes for reader-writer locks.
- Circular buffer to simulate producer-consumer interaction.

---

## Future Enhancements

- Extend to multiple producers and consumers for Part 1.
- Dynamic input for buffer size and thread counts.
- Add timing analysis for performance metrics.