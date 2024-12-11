# Virtual Memory Management System Simulation

This project implements a simulation of **Translation Lookaside Buffer (TLB)** replacement algorithms within a virtual memory management system. The simulation demonstrates the behavior of the following TLB replacement algorithms:

- **FIFO (First-In-First-Out)**: Replaces the oldest entry in the TLB.
- **LIFO (Last-In-First-Out)**: Replaces the most recently added entry.
- **LRU (Least Recently Used)**: Replaces the least recently accessed entry.
- **Optimal Algorithm**: Replaces the entry that will not be used for the longest time in the future.

Additionally, a custom memory management system using `mmap` is implemented to avoid the direct usage of `malloc` and `calloc`.

---

## Key Components

### **Custom Memory Management**
- **`my_malloc(size_t size)`**: Allocates memory using `mmap`.
- **`my_calloc(size_t nelem, size_t size)`**: Allocates and initializes memory to zero using `mmap`.
- **`my_free(void* ptr)`**: Releases memory allocated using `my_malloc` or `my_calloc`.

### **TLB Replacement Algorithms**
1. **FIFO**:
   - Implements a queue to track the order of TLB entries.
   - Replaces the oldest entry when the TLB is full.

2. **LIFO**:
   - Implements a stack to track TLB entries.
   - Replaces the most recently added entry when the TLB is full.

3. **LRU**:
   - Tracks the access time of each entry.
   - Replaces the least recently accessed entry.

4. **Optimal**:
   - Uses future memory access patterns to identify the entry not used for the longest time.
   - Provides the lowest miss rate but requires knowledge of future accesses.

---

## Input Format

1. **Test Cases**:  
   - The first line contains an integer `T` (1 ≤ T ≤ 100), representing the number of test cases.

2. **Per Test Case Input**:
   - **Address Space Size**: Integer `S` (1 ≤ S ≤ 4096), in megabytes (MB).
   - **Page Size**: Integer `P` (1 ≤ P ≤ S), in kilobytes (KB).
   - **TLB Size**: Integer `K` (1 ≤ K ≤ 1024), the number of entries in the TLB.
   - **Number of Memory Accesses**: Integer `N` (1 ≤ N ≤ 10^6).
   - **Memory Addresses**: `N` lines of 32-bit hexadecimal addresses without prefix (e.g., `1A2B3C4D`).

---

## Output Format

For each test case, the program outputs the number of TLB hits for each algorithm in the following format:

Where `X` is the test case number, and `Y1`, `Y2`, `Y3`, `Y4` are the respective hit counts for the algorithms.

---

## Compilation and Execution

### **Requirements**
- C++ Compiler (e.g., `g++`).
- POSIX-compliant system.

### **Commands**
1. **Compile the Program**
   ```bash
   g++ -o virtual_memory_simulation virtual_memory_simulation.cpp