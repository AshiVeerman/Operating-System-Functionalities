#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

// Function to execute standard commands using execvp
void std_commands(char *command) {
    char *args[2048];
    char *token = strtok(command, "\n \t");
    int i = 0;

    while (token != NULL) {
        args[i++] = token;
        token = strtok(NULL, "\n \t");
    }
    args[i] = NULL;

    if (execvp(args[0], args) == -1) {
        printf("Invalid Command\n");
    }
    exit(EXIT_FAILURE);
}

// Function to trim leading and trailing whitespace from a string
void trim(char *str) {
    int start = 0;
    while (str[start] == ' ' || str[start] == '\t') start++;

    int end = strlen(str) - 1;
    while (end >= start && (str[end] == ' ' || str[end] == '\n' || str[end] == '\t')) end--;

    memmove(str, str + start, end - start + 1);
    str[end - start + 1] = '\0';
}

int main() {
    char command[2048];
    char prev_dir[2048] = "";
    char curr_dir[2048];
    char *history[100];
    int num_cmds = 0;

    while (1) {
        printf("MTL458 > ");

        if (!fgets(command, sizeof(command), stdin)) break;

        // Save the command in history
        if (num_cmds < 100) {
            history[num_cmds++] = strdup(command);
        } else {
            free(history[0]);
            memmove(history, history + 1, (99) * sizeof(char *));
            history[99] = strdup(command);
        }

        trim(command);

        // Exit command
        if (strcmp(command, "exit") == 0) break;

        // History command
        if (strcmp(command, "history") == 0) {
            for (int i = 0; i < num_cmds; i++) {
                printf("%s", history[i]);
            }
            continue;
        }

        // Pipe handling
        char *pipe_loc = strchr(command, '|');
        if (pipe_loc != NULL) {
            *pipe_loc = '\0';
            char *cmd1 = command;
            char *cmd2 = pipe_loc + 1;

            trim(cmd1);
            trim(cmd2);

            int fd[2];
            if (pipe(fd) == -1) {
                printf("Invalid Command\n");
                continue;
            }

            if (fork() == 0) {
                dup2(fd[1], STDOUT_FILENO);
                close(fd[0]);
                close(fd[1]);
                std_commands(cmd1);
            }

            if (fork() == 0) {
                dup2(fd[0], STDIN_FILENO);
                close(fd[0]);
                close(fd[1]);
                std_commands(cmd2);
            }

            close(fd[0]);
            close(fd[1]);
            wait(NULL);
            wait(NULL);
            continue;
        }

        // Change directory (cd) command
        if (strncmp(command, "cd ", 3) == 0) {
            char *dir = command + 3;
            trim(dir);

            if (*dir == '~') {
                dir = getenv("HOME");
                if (chdir(dir) != 0) printf("Invalid Command\n");
            } else if (*dir == '-') {
                if (strlen(prev_dir) == 0) {
                    printf("Invalid Command\n");
                } else {
                    if (chdir(prev_dir) == 0 && getcwd(curr_dir, sizeof(curr_dir)) != NULL) {
                        printf("%s\n", curr_dir);
                        strncpy(prev_dir, curr_dir, sizeof(prev_dir));
                    } else {
                        printf("Invalid Command\n");
                    }
                }
            } else {
                if (getcwd(curr_dir, sizeof(curr_dir)) != NULL) strncpy(prev_dir, curr_dir, sizeof(prev_dir));
                if (chdir(dir) != 0) printf("Invalid Command\n");
            }
            continue;
        }

        // Standard command execution
        if (fork() == 0) std_commands(command);
        else wait(NULL);
    }

    // Free allocated memory for history
    for (int i = 0; i < num_cmds; i++) {
        free(history[i]);
    }

    return 0;
}