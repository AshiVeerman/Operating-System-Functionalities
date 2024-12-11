#pragma once

//Can include any other headers as needed
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/time.h>
#include <stdbool.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>

// time to get curr time
uint64_t absolute_current_time() {
    struct timeval time_val;
    gettimeofday(&time_val, NULL);
    uint64_t ms = (uint64_t)(time_val.tv_sec) * 1000 + (uint64_t)(time_val.tv_usec) / 1000;
    return ms;
}

typedef struct {
    //This will be given by the tester function this is the process command to be scheduled
    char *command;
    //Temporary parameters for your usage can modify them as you wish
    bool finished;  //If the process is finished safely
    bool error;    //If an error occurs during execution
    uint64_t start_time;
    uint64_t completion_time;
    uint64_t turnaround_time;
    uint64_t waiting_time;
    uint64_t response_time;
    bool started; 
    int process_id;
    pid_t pid; 
    int priority; // for storing priority for mlfq
} Process;

// Function prototypes
void FCFS(Process p[], int n);
void RoundRobin(Process p[], int n, int quantum);
void MultiLevelFeedbackQueue(Process p[], int n, int quantum0, int quantum1, int quantum2, int boostTime);

void FCFS(Process p[], int n) {
    uint64_t arrival_time = absolute_current_time();
    for (int i = 0; i < n; i++) {
        p[i].start_time = absolute_current_time();
        p[i].started = true;

        pid_t pid = fork();
        if (pid == 0) {
            char command_copy[2048];
            strncpy(command_copy, p[i].command, sizeof(command_copy));
            char *args[100];
            char *token = strtok(command_copy, " ");
            int arg_idx = 0;
            while (token != NULL) {
                args[arg_idx++] = token;
                token = strtok(NULL, " ");
            }
            args[arg_idx] = NULL;
            if (execvp(args[0], args) == -1) {
                p[i].error=true;
                exit(EXIT_FAILURE);
            }
        } else if (pid > 0) {
            int status;
            waitpid(pid, &status, 0);
            if (WIFEXITED(status)) {
                if (WEXITSTATUS(status)!=0) p[i].error=true;
            }
            uint64_t curr_time = absolute_current_time();
            p[i].completion_time = curr_time-p[i].start_time;
            p[i].turnaround_time = curr_time - arrival_time;
            p[i].waiting_time = p[i].start_time - arrival_time;
            p[i].response_time = p[i].waiting_time;
            p[i].finished = true;
        } else {
            p[i].error = true;
        }
        printf("%s|%llu|%llu\n",p[i].command,p[i].waiting_time,p[i].turnaround_time);
    }

    // Open the file in write mode
    FILE *file = fopen("result_offline_FCFS.csv", "w");
    fprintf(file,"Command,Finished,Error,Burst Time,Turnaround Time,Waiting Time,Response Time\n");
    for (int i = 0; i < n; i++){
        char *fin = (p[i].finished==0)? "No":"Yes";
        char *err = (p[i].error==0)? "No":"Yes";
        if (strcmp(err,"Yes")==0) fin="No";
        fprintf(file, "%s,%s,%s,%llu,%llu,%llu,%llu\n",p[i].command,fin,err,p[i].completion_time,p[i].turnaround_time,p[i].waiting_time,p[i].response_time);
    }
    fclose(file);
}

void RoundRobin(Process p[], int n, int quantum) {
    int processes_left = n; 
    uint64_t arrival_time = absolute_current_time();
    uint64_t start_ctxt,end_ctxt;
    for (int i=0;i<n;i++) p[i].completion_time=0;
    while (processes_left > 0) {
        for (int i = 0; i < n; i++) {
            if (p[i].finished) {
                continue;
            }
            if (!p[i].started) {
                pid_t pid = fork();
                if (pid == 0) { 
                    char *args[100];
                    char command_copy[2048];
                    strncpy(command_copy, p[i].command, sizeof(command_copy));
                    char *token = strtok(command_copy, " ");
                    int arg_idx = 0;
                    while (token != NULL) {
                        args[arg_idx++] = token;
                        token = strtok(NULL, " ");
                    }
                    args[arg_idx] = NULL;
                    if (execvp(args[0], args) == -1) {
                        exit(EXIT_FAILURE);
                    }
                } else if (pid > 0) {
                    p[i].pid = pid;
                    p[i].start_time = absolute_current_time();
                    start_ctxt = p[i].start_time;
                    p[i].started = true;
                } else {
                    p[i].error = true;
                    continue;
                }
            } else {
                start_ctxt = absolute_current_time();
                kill(p[i].pid, SIGCONT);
            }
            usleep(quantum * 1000);
            int status;
            int result = waitpid(p[i].pid, &status, WNOHANG);
            if ( result == 0) {
                kill(p[i].pid, SIGSTOP);
                waitpid(p[i].pid, NULL, WUNTRACED);
                end_ctxt = absolute_current_time();
                //printf("Process %d paused\n", i+1);
                p[i].completion_time += (end_ctxt-start_ctxt);
                printf("%s|%llu|%llu\n",p[i].command,start_ctxt-arrival_time,end_ctxt-arrival_time);
            } else {
                // Process finished during this quantum
                uint64_t curr_time = absolute_current_time();
                p[i].completion_time += (curr_time-start_ctxt);
                p[i].turnaround_time = curr_time - arrival_time;
                p[i].waiting_time = p[i].turnaround_time - p[i].completion_time;
                p[i].response_time = p[i].start_time-arrival_time;
                p[i].finished = true;
                processes_left--;
                //printf("Process %d finished\n", i+1);
                printf("%s|%llu|%llu\n",p[i].command,start_ctxt-arrival_time,p[i].turnaround_time);
            }
        }
    }
    // Open the file in write mode
    FILE *file = fopen("result_offline_RR.csv", "w");
    fprintf(file,"Command,Finished,Error,Burst Time,Turnaround Time,Waiting Time,Response Time\n");
    for (int i = 0; i < n; i++){
        char *fin = (p[i].finished==0)? "No":"Yes";
        char *err = (p[i].error==0)? "No":"Yes";
        if (strcmp(err,"Yes")==0) fin="No";
        fprintf(file, "%s,%s,%s,%llu,%llu,%llu,%llu\n",p[i].command,fin,err,p[i].completion_time,p[i].turnaround_time,p[i].waiting_time,p[i].response_time);
    }
    fclose(file);
}

// Multi-Level Feedback Queue implementation
void MultiLevelFeedbackQueue(Process p[], int n, int quantum0, int quantum1, int quantum2, int boostTime) {
    int processes_left = n;
    int quantum[3] = {quantum0, quantum1, quantum2}; 
    int queue_count = 3;
    uint64_t arrival_time = absolute_current_time();
    uint64_t last_boost_time = arrival_time;
    uint64_t start_ctxt,end_ctxt;
    bool flag=0;
    while (processes_left > 0) {
        bool process_executed = false;
        uint64_t current_time = absolute_current_time();
        for (int queue = 0; queue < queue_count; queue++) {
            for (int i = 0; i < n; i++) {
                if (p[i].finished || p[i].priority != queue) {
                    continue;
                }
                if (!p[i].started) {
                    pid_t pid = fork();
                    if (pid == 0) {
                        char *args[10];
                        char command_copy[256];
                        strncpy(command_copy, p[i].command, sizeof(command_copy));

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
                        p[i].pid = pid;
                        p[i].start_time = absolute_current_time();
                        start_ctxt = p[i].start_time;
                        p[i].started = true;
                        //printf("Process %d started in queue %d\n", i + 1, queue);
                    } else {
                        p[i].error = true;
                        continue;
                    }
                } else {
                    start_ctxt = absolute_current_time();
                    //printf("Process %d started in queue %d\n", i + 1, queue);
                    kill(p[i].pid, SIGCONT);
                }
                usleep(quantum[queue] * 1000);

                // After the quantum has passed, check if the process is finished
                int status;
                int result = waitpid(p[i].pid, &status, WNOHANG);
                if (result == 0) {
                    // If the process is not finished, pause it
                    kill(p[i].pid, SIGSTOP);  
                    waitpid(p[i].pid, NULL, WUNTRACED); 
                    end_ctxt = absolute_current_time();
                    p[i].completion_time += (end_ctxt-start_ctxt);
                    printf("%s|%llu|%llu\n",p[i].command,start_ctxt-arrival_time,end_ctxt-arrival_time);
                    if (queue < 2) {
                        p[i].priority++;
                        //printf("Process %d demoted to queue %d\n", i + 1, p[i].priority);
                    }
                } else {
                    // Process finished during this quantum
                    uint64_t curr_time = absolute_current_time();
                    p[i].completion_time += (curr_time-start_ctxt);
                    p[i].turnaround_time = curr_time-arrival_time;
                    p[i].waiting_time = p[i].turnaround_time-p[i].completion_time;
                    p[i].response_time = p[i].start_time-arrival_time;
                    p[i].finished = true;
                    processes_left--;
                    //printf("Process %d finished\n", i+1);
                    printf("%s|%llu|%llu\n",p[i].command,start_ctxt-arrival_time,p[i].turnaround_time);
                }

                // Check for boost time after each process
                current_time = absolute_current_time();
                if (current_time - last_boost_time >= boostTime) {
                    for (int j = 0; j < n; j++) {
                        if (!p[j].finished && p[j].priority > 0) {
                            p[j].priority=0;
                            //printf("Process %d boosted to queue %d\n", j + 1, p[j].priority);
                        }
                    }
                    last_boost_time = current_time;
                }
            }
        }
    }
    // Open the file in write mode
    FILE *file = fopen("result_offline_MLFQ.csv", "w");
    fprintf(file,"Command,Finished,Error,Burst Time,Turnaround Time,Waiting Time,Response Time\n");
    for (int i = 0; i < n; i++){
        char *fin = (p[i].finished==0)? "No":"Yes";
        char *err = (p[i].error==0)? "No":"Yes";
        if (strcmp(err,"Yes")==0) fin="No";
        fprintf(file, "%s,%s,%s,%llu,%llu,%llu,%llu\n",p[i].command,fin,err,p[i].completion_time,p[i].turnaround_time,p[i].waiting_time,p[i].response_time);
    }
    fclose(file);
}