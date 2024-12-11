# Offline and Online Schedulers

This project implements scheduling algorithms for process management, simulating both **offline** and **online** schedulers. Each scheduler demonstrates how processes are scheduled based on various algorithms, focusing on performance metrics such as turnaround time, waiting time, and response time.

---

## File Structure
### **Offline Schedulers**
- Implements:
  - First-Come-First-Serve (FCFS)
  - Round Robin (RR)
  - Multi-Level Feedback Queue (MLFQ)
- File: `offline_schedulers.c`

### **Online Schedulers**
- Implements:
  - Shortest Job First (SJF)
  - Multi-Level Feedback Queue (MLFQ)
- File: `online_schedulers.c`

---

## Key Data Structure: `Process`
The `Process` struct is central to all scheduling algorithms and contains:

| Field                  | Description                                                                                          |
|------------------------|------------------------------------------------------------------------------------------------------|
| `command`              | The process command to be executed.                                                                 |
| `finished`             | A boolean flag indicating whether the process completed successfully.                                |
| `error`                | A boolean flag indicating whether an error occurred during execution.                                |
| `start_time`           | The absolute start time of the process (in milliseconds).                                           |
| `completion_time`      | The absolute completion time of the process (in milliseconds).                                      |
| `turnaround_time`      | The total time taken to complete the process (`completion_time - start_time`).                      |
| `waiting_time`         | Time the process spent waiting in the ready queue.                                                  |
| `response_time`        | Time from when the process was submitted to when it started execution.                              |
| `started`              | A boolean flag indicating whether the process has started execution.                                |
| `process_id`           | The unique identifier for the process.                                                              |
| `pid`                  | Process ID of the forked child (for process management).                                             |
| `priority`             | (Used in MLFQ) Priority level of the process, determining its queue placement.                       |

---

## Offline Schedulers
Offline schedulers schedule all processes before execution begins. 

### **Algorithms**
1. **First-Come-First-Serve (FCFS)**
   - Processes are executed in the order they arrive.
   - Non-preemptive.

2. **Round Robin (RR)**
   - Processes are executed in a circular manner with a fixed time quantum.
   - Preemptive: If a process exceeds the quantum, it is moved to the back of the queue.

3. **Multi-Level Feedback Queue (MLFQ)**
   - Processes are assigned to queues with varying priority levels.
   - Each queue has its own time quantum.
   - Aging and boosting mechanisms ensure fair scheduling.

---

## Online Schedulers
Online schedulers make scheduling decisions dynamically as processes arrive.

### **Algorithms**
1. **Shortest Job First (SJF)**
   - Selects the process with the shortest execution time.
   - Preemptive version schedules the next process if a shorter one arrives.

2. **Multi-Level Feedback Queue (MLFQ)**
   - Similar to the offline implementation but dynamically adjusts queues based on process behavior.

---

## Key Functions

### **Utility Function: `absolute_current_time`**
- Returns the current time in milliseconds.
- Used to calculate timing metrics for each process.

### **Offline Scheduler Functions**
| Function                             | Description                                                                 |
|--------------------------------------|-----------------------------------------------------------------------------|
| `void FCFS(Process p[], int n)`      | Implements the First-Come-First-Serve scheduling algorithm.                 |
| `void RoundRobin(Process p[], int n, int quantum)` | Implements the Round Robin scheduling algorithm.                              |
| `void MultiLevelFeedbackQueue(Process p[], int n, int quantum0, int quantum1, int quantum2, int boostTime)` | Implements the MLFQ scheduling algorithm. |

### **Online Scheduler Functions**
| Function                                      | Description                                                                 |
|-----------------------------------------------|-----------------------------------------------------------------------------|
| `void ShortestJobFirst()`                     | Implements the Shortest Job First scheduling algorithm.                     |
| `void MultiLevelFeedbackQueue(int q0, int q1, int q2, int boost)` | Implements the MLFQ scheduling algorithm.                                   |

---

## Compilation and Execution

### **Requirements**
- C Compiler (e.g., `gcc`)
- POSIX-compliant environment

### **Commands**

1. **Compile Offline Scheduler**
   ```bash
   gcc -o offline_schedulers offline_schedulers.c