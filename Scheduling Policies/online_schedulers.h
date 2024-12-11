#pragma once

//Can include any other headers as needed
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/time.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

typedef struct {
    char *command;
    bool finished;
    bool error;    
    uint64_t start_time;
    uint64_t completion_time;
    uint64_t turnaround_time;
    uint64_t waiting_time;
    uint64_t response_time;
    bool started; 
    int process_id;
    int priority;
    pid_t pid;
} Process;

// Function prototypes
void ShortestJobFirst();
void ShortestRemainingTimeFirst(); // yeh nhi karna ab wow
void MultiLevelFeedbackQueue(int quantum0, int quantum1, int quantum2, int boostTime);

// time to get curr time
uint64_t absolute_current_time() {
    struct timeval time_val;
    gettimeofday(&time_val, NULL);
    uint64_t ms = (uint64_t)(time_val.tv_sec) * 1000 + (uint64_t)(time_val.tv_usec) / 1000;
    return ms;
}

// Function to execute a command
uint64_t sjf_helper(char *command, Process *p) {
    pid_t pid = fork();
    if (pid == 0) {
        char *args[100];
        char command_copy[2048];
        strncpy(command_copy, command, sizeof(command_copy));
        command_copy[sizeof(command_copy) - 1] = '\0';
        char *token = strtok(command_copy, " ");
        int arg_idx = 0;
        while (token != NULL) {
            args[arg_idx++] = token;
            token = strtok(NULL, " ");
        }
        args[arg_idx] = NULL; 
        execvp(args[0], args);
        exit(EXIT_FAILURE);
    } else if (pid > 0) {  
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            if (WEXITSTATUS(status)!=0) p->error=true;
            return WEXITSTATUS(status); 
        } else {
            p->error=true;
            return 1;
        }
    } else {
        p->error=true;
        return 1;
    }
    return 1;
}

// map kinda structure to store number of such commands executed, index, and burst time
typedef struct {
    char command[2048];
    int index;
    uint64_t burst_time;
    int count;
} CommandMapEntry;

CommandMapEntry cmd_map[50];
int m_size = 0;

int find_cmd(char *command) {
    for (int i = 0; i < m_size; i++) {
        if (strcmp(cmd_map[i].command, command) == 0) {
            return cmd_map[i].index;
        }
    }
    return -1;
}

void add_cmd(char *command, int index) {
    if (m_size < 50) {
        strncpy(cmd_map[m_size].command, command, 2048);
        cmd_map[m_size].command[2048 - 1] = '\0';
        cmd_map[m_size].index = index;
        cmd_map[m_size].burst_time=1000;
        cmd_map[m_size].count=0;
        m_size++;
    }
}

// Some issue with this
// // Non-blocking input reader setup using fcntl
// void setup_non_blocking_input() {
//     int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
//     fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
// }

// Shortest Job First scheduling algorithm using non-blocking input
void ShortestJobFirst() {
    char command[2048];
    Process processes[50];
    int num_processes = 0;
    uint64_t burst_times[50]; // Not really using this anywhere redundant hai
    int process_count = 0; // not used unnecessary var
    char buffer[2048];
    FILE *file = fopen("result_online_SJF.csv", "w");
    fprintf(file,"Command,Finished,Error,Burst Time,Turnaround Time,Waiting Time,Response Time\n");
    fflush(file); 
    //setup_non_blocking_input();
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
    uint64_t arrival_time = absolute_current_time();
    while (1) {
        while (fgets(buffer,sizeof(buffer),stdin)){
            buffer[strcspn(buffer,"\n")]=0;
            if (strcmp(buffer,"exit")==0){
                exit(0);
            }
            uint64_t curr_time = absolute_current_time();
            processes[num_processes].start_time = curr_time;

            // if the command already exists in the cmd_map
            int index = find_cmd(buffer);
            if (index == -1) {
                processes[num_processes].completion_time = 0;
                processes[num_processes].command = strdup(buffer);
                processes[num_processes].started = false;
                processes[num_processes].finished = false;
                processes[num_processes].error = false;
                processes[num_processes].process_id = num_processes;
                burst_times[num_processes] = 1;
                add_cmd(buffer, num_processes);
                num_processes++;
            } else {
                // Existing command, update burst time
                processes[num_processes].completion_time = 0;
                processes[num_processes].command = strdup(buffer);
                processes[num_processes].started = false;
                processes[num_processes].finished = false;
                processes[num_processes].error = false;
                processes[num_processes].process_id = num_processes;
                burst_times[num_processes] = cmd_map[index].burst_time;
                num_processes++;
            }
        }
        if (process_count < num_processes) {
            int min_val_idx = -1;
            uint64_t min_burst_time = UINT64_MAX;
            for (int i = 0; i < num_processes; i++) {
                int idx = find_cmd(processes[i].command);
                if (!processes[i].finished && cmd_map[idx].burst_time < min_burst_time) {
                    min_burst_time = cmd_map[idx].burst_time;
                    min_val_idx = i;
                }
            }

            if (min_val_idx!= -1) {
                // Execute the shortest process
                uint64_t start_time = absolute_current_time();
                burst_times[min_val_idx] = sjf_helper(processes[min_val_idx].command,&processes[min_val_idx]);
                uint64_t end_time = absolute_current_time();
                processes[min_val_idx].completion_time = end_time-start_time;
                processes[min_val_idx].turnaround_time = end_time - processes[min_val_idx].start_time;
                processes[min_val_idx].waiting_time = processes[min_val_idx].turnaround_time - processes[min_val_idx].completion_time;
                processes[min_val_idx].response_time = processes[min_val_idx].waiting_time;
                processes[min_val_idx].finished = true;
                process_count++;
                printf("%s|%llu|%llu\n",processes[min_val_idx].command,start_time-arrival_time,end_time-arrival_time);
                fflush(stdout);
                const char *fin = (processes[min_val_idx].finished==0)? "No":"Yes";
                const char *err = (processes[min_val_idx].error==0)? "No":"Yes";
                if (strcmp(err,"Yes")==0) fin="No";
                fprintf(file,"%s,%s,%s,%llu,%llu,%llu,%llu\n",processes[min_val_idx].command,fin,err,processes[min_val_idx].completion_time,processes[min_val_idx].turnaround_time,processes[min_val_idx].waiting_time,processes[min_val_idx].response_time);
                // Update burst times for the next round
                fflush(file); 
                int index = find_cmd(processes[min_val_idx].command);
                if (cmd_map[index].count==0){
                    cmd_map[index].burst_time=processes[min_val_idx].completion_time;
                }
                else{
                    cmd_map[index].burst_time = (cmd_map[index].burst_time*cmd_map[index].count + processes[min_val_idx].completion_time)/(cmd_map[index].count+1);
                }
                cmd_map[index].count+=1;
                //printf("%s,%llu",processes[shortest].command,processes[shortest].completion_time);
            }
        }
        usleep(100000);  // Sleep for 100 ms
    }
    fclose(file);

    for (int i = 0; i < num_processes; i++) {
        free(processes[i].command);
    }
}

// Queues for each priority level
Process q0[50];
Process q1[50];
Process q2[50];
int q0_size = 0, q1_size = 0, q2_size = 0;

// Function to execute a process for a specific quantum
void execute(Process *p, int quantum, uint64_t arrival_time) {
    uint64_t start_ctxt,end_ctxt;
    if (!p->started) {
        pid_t pid = fork(); 
        if (pid == 0) { 
            char *args[100]; 
            char command_copy[2048]; 
            strncpy(command_copy, p->command, sizeof(command_copy));
            char *token = strtok(command_copy, " ");
            int arg_idx = 0;
            while (token != NULL) {
                args[arg_idx++] = token;
                token = strtok(NULL, " ");
            }
            args[arg_idx] = NULL;  
            if (execvp(args[0], args) == -1) {
                exit(errno);
            }
        } else if (pid > 0) { 
            p->pid = pid; 
            p->start_time = absolute_current_time();
            start_ctxt = p->start_time;
            p->started = true;
            //printf("Process %d started in queue %d\n", i + 1, queue);
        } else {
            p->error = true;
        }
    } else {
        start_ctxt = absolute_current_time();
        //printf("Process started in queue %d\n", p->priority);
        kill(p->pid, SIGCONT);
    }
    usleep(quantum * 1000); 
    int status;
    int result = waitpid(p->pid, &status, WNOHANG);
    if (result == 0) {
        kill(p->pid, SIGSTOP);  
        waitpid(p->pid, NULL, WUNTRACED); 
        end_ctxt = absolute_current_time();
        p->completion_time += (end_ctxt-start_ctxt);
        printf("%s|%llu|%llu\n",p->command,start_ctxt-arrival_time,end_ctxt-arrival_time);
        fflush(stdout);
        if (p->priority < 2) {
            p->priority+=1;
            //printf("Process demoted to queue %d\n", p->priority);
        }
    } else {
        // Process finished during this quantum
        uint64_t curr_time = absolute_current_time();
        p->completion_time += (curr_time-start_ctxt);
        p->turnaround_time = curr_time-arrival_time;
        p->waiting_time = p->turnaround_time-p->completion_time;  // Simplified waiting time calculation
        p->response_time = p->start_time-arrival_time;
        p->finished = true;
        //printf("Process finished\n");
        printf("%s|%llu|%llu\n",p->command,start_ctxt-arrival_time,p->turnaround_time);
        fflush(stdout);
    }
}


void MultiLevelFeedbackQueue(int quantum0, int quantum1, int quantum2, int boostTime){
    uint64_t arrival_time = absolute_current_time();
    uint64_t last_boost_time = arrival_time;
    int num_processes = 0;
    char command[2048];
    Process processes[50];
    FILE *file = fopen("result_online_MLFQ.csv", "w");
    fprintf(file,"Command,Finished,Error,Burst Time,Turnaround Time,Waiting Time,Response Time\n");
    fflush(file); 
    //setup_non_blocking_input();
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
    while (1) {
        uint64_t curr_time = absolute_current_time();
        
        // Check for priority boost
        if (curr_time - last_boost_time >= boostTime) {
            for (int i=0;i<q1_size;i++){
                q0[q0_size++]=q1[i];
            }
            for (int i=0;i<q2_size;i++){
                q0[q0_size++]=q2[i];
            }
            q1_size=0;
            q2_size=0;
            last_boost_time = curr_time;
        }

        // Non-blocking input to add new processes
        char buffer[2048];
        while (fgets(buffer,sizeof(buffer),stdin)){
            buffer[strcspn(buffer,"\n")]=0;
            if (strcmp(buffer,"exit")==0){
                exit(0);
            }
            uint64_t curr_time = absolute_current_time();
            processes[num_processes].start_time = curr_time;

            // Check if the command already exists in the cmd_map
            int index = find_cmd(buffer);
            if (index == -1) {
                processes[num_processes].command = strdup(buffer);
                processes[num_processes].started = false;
                processes[num_processes].finished = false;
                processes[num_processes].error = false;
                processes[num_processes].process_id = num_processes;
                processes[num_processes].priority=1;
                processes[num_processes].completion_time=0;
                q1[q1_size++]=processes[num_processes];
                add_cmd(buffer, num_processes);
                num_processes++;
            } else {
                processes[num_processes].command = strdup(buffer);
                processes[num_processes].started = false;
                processes[num_processes].finished = false;
                processes[num_processes].error = false;
                processes[num_processes].process_id = num_processes;
                processes[num_processes].completion_time=0;
                if (cmd_map[index].burst_time<=quantum0) {
                    processes[num_processes].priority=0;
                    q0[q0_size++]=processes[num_processes];
                }
                else if (cmd_map[index].burst_time<=quantum1){
                    processes[num_processes].priority=1;
                    q1[q1_size++]=processes[num_processes];
                }
                else {
                    processes[num_processes].priority=2;
                    q2[q2_size++]=processes[num_processes];
                }
                num_processes++;
            }
        }
        if (q0_size > 0) {  // Process one command from q0
            execute(&q0[0],quantum0,arrival_time);
            if (q0[0].priority!=0) q1[q1_size++]=q0[0];
            if (q0[0].finished){
                const char *fin = (q0[0].finished==0)? "No":"Yes";
                const char *err = (q0[0].error==0)? "No":"Yes";
                if (strcmp(err,"Yes")==0) fin="No";
                fprintf(file,"%s,%s,%s,%llu,%llu,%llu,%llu\n",q0[0].command,fin,err,q0[0].completion_time,q0[0].turnaround_time,q0[0].waiting_time,q0[0].response_time);
                fflush(file); 
                int index = find_cmd(q0[0].command);
                if (cmd_map[index].count==0){
                    cmd_map[index].burst_time=q0[0].completion_time;
                }
                else{
                    cmd_map[index].burst_time = (cmd_map[index].burst_time*cmd_map[index].count + q0[0].completion_time)/(cmd_map[index].count+1);
                }
                cmd_map[index].count+=1;
            }
            for (int i = 1; i < q0_size; i++) {
                q0[i - 1] = q0[i];
            }
            q0_size--;
        } else if (q1_size > 0) { 
            execute(&q1[0],quantum1,arrival_time);
            if (q1[0].finished){
                const char *fin = (q1[0].finished==0)? "No":"Yes";
                const char *err = (q1[0].error==0)? "No":"Yes";
                if (strcmp(err,"Yes")==0) fin="No";
                fprintf(file,"%s,%s,%s,%llu,%llu,%llu,%llu\n",q1[0].command,fin,err,q1[0].completion_time,q1[0].turnaround_time,q1[0].waiting_time,q1[0].response_time);
                fflush(file); 
                int index = find_cmd(q1[0].command);
                if (cmd_map[index].count==0){
                    cmd_map[index].burst_time=q1[0].completion_time;
                }
                else{
                    cmd_map[index].burst_time = (cmd_map[index].burst_time*cmd_map[index].count + q1[0].completion_time)/(cmd_map[index].count+1);
                }
                cmd_map[index].count+=1;
            }
            //printf("%d\n",q1[0].priority);
            if (q1[0].priority!=1) q2[q2_size++]=q1[0];
            for (int i = 1; i < q1_size; i++) {
                q1[i - 1] = q1[i];
            }
            q1_size--;
        } else if (q2_size > 0) {  // Process one command from q2
            execute(&q2[0],quantum2,arrival_time);
            // Shift remaining elements in q2
            if (q2[0].finished){
                const char *fin = (q2[0].finished==0)? "No":"Yes";
                const char *err = (q2[0].error==0)? "No":"Yes";
                if (strcmp(err,"Yes")==0) fin="No";
                fprintf(file,"%s,%s,%s,%llu,%llu,%llu,%llu\n",q2[0].command,fin,err,q2[0].completion_time,q2[0].turnaround_time,q2[0].waiting_time,q2[0].response_time);
                // Update burst times for the next round
                fflush(file); 
                int index = find_cmd(q2[0].command);
                if (cmd_map[index].count==0){
                    cmd_map[index].burst_time=q2[0].completion_time;
                }
                else{
                    cmd_map[index].burst_time = (cmd_map[index].burst_time*cmd_map[index].count + q2[0].completion_time)/(cmd_map[index].count+1);
                }
                cmd_map[index].count+=1;
                for (int i = 1; i < q2_size; i++) {
                    q2[i - 1] = q2[i];
                }
                q2_size--;
            }
            //printf("%d",q2_size);
            fflush(stdout);
        }
        //printf("%d,%d,%d\n",q0_size,q1_size,q2_size);
        usleep(1000000);  // Sleep for 1s
        //printf("%llu\n",absolute_current_time()-arrival_time);
    }
    fclose(file);
}