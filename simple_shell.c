#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <errno.h>

// Structure for storing command history
struct History {
    char commands[20][1200];
    int count;
};

// Structure for storing PID, execution time, and duration
struct History2 {
    pid_t pids[20];
    time_t execution_time[20];
    double execution_duration[20];
    int count;
};

// Function to remove the newline character from a string
void remove_newline(char *str) {
    str[strcspn(str, "\n")] = '\0';
}

// Function to add a command to the history
void add_to_history(struct History *history, char *command) {
    if (history->count < 20) {
        strncpy(history->commands[history->count], command, 1199);
        history->commands[history->count][1199] = '\0'; // Ensure null-termination
        history->count++;
    } else {
        // If the history is full, remove the oldest command
        for (int i = 0; i < 19; i++) {
            strncpy(history->commands[i], history->commands[i + 1], 1200);
        }
        strncpy(history->commands[19], command, 1199);
        history->commands[19][1199] = '\0'; // Ensure null-termination
    }
}

// Function to parse the command and its arguments
void parse_command(char *command, char **args) {
    int i = 0;
    args[i] = strtok(command, " ");
    while (args[i] != NULL && i < 99) { // Limit to 99 arguments to prevent buffer overflow
        i++;
        args[i] = strtok(NULL, " ");
    }
    args[i] = NULL; // Ensure the last element is NULL for execvp
}

// Updated function to check for invalid characters in the command
int is_valid_command(char *command) {
    // Allow most characters, but still prevent obvious security risks
    return (strchr(command, ';') == NULL && strchr(command, '&') == NULL);
}

// Updated function to execute a single command and record its PID, execution time, and duration
char* execute_command(char *command, struct History *history, struct History2 *history2) {
    char* result = (char*)malloc(1201);  // +1 for null terminator
    if (result == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    // Check if the command ends with "&"
    int run_in_background = 0;
    int command_length = strlen(command);
    if (command_length > 1 && command[command_length - 1] == '&') {
        run_in_background = 1;
        // Remove the "&" from the command
        command[command_length - 1] = '\0';
    }

    // Add the command to history
    add_to_history(history, command);

    // Record the start time
    time_t start_time = time(NULL);

    // Parse the command and its arguments
    char *args[100];
    parse_command(command, args);

    // Fork a child process to execute the command
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork failed");
        snprintf(result, 1200, "Error: Failed to create child process\n");
    } else if (pid == 0) {
        // Child process
        char *path = getenv("PATH");
        char *path_copy = strdup(path);
        char *dir = strtok(path_copy, ":");
        char full_path[1024];

        while (dir != NULL) {
            snprintf(full_path, sizeof(full_path), "%s/%s", dir, args[0]);
            execv(full_path, args);
            dir = strtok(NULL, ":");
        }

        free(path_copy);
        fprintf(stderr, "Command not found: %s\n", args[0]);
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        // Record the PID and execution start time
        if (history2->count < 20) {
            history2->pids[history2->count] = pid;
            history2->execution_time[history2->count] = start_time;
        }

        if (run_in_background == 0) {
            int status;
            if (waitpid(pid, &status, 0) == -1) {
                perror("waitpid failed");
                snprintf(result, 1200, "Error: Failed to wait for child process\n");
            } else {
                if (!WIFEXITED(status)) {
                    snprintf(result, 1200, "%s\tCommand did not exit normally\n", command);
                } else {
                    // Record the execution duration
                    time_t end_time = time(NULL);
                    double duration = difftime(end_time, start_time);
                    if (history2->count < 20) {
                        history2->execution_duration[history2->count] = duration;
                    }

                    snprintf(result, 1200, "Command executed successfully\n");
                }
            }
        } else {
            // If the command is running in the background, don't wait for it
            snprintf(result, 1200, "%s\tCommand running in the background with PID: %d\n", command, pid);
        }

        if (history2->count < 20) {
            history2->count++;
        }
    }

    return result;
}

// Function to execute a command with pipes
void execute_piped_commands(char *command, struct History *history, struct History2 *history2) {
    add_to_history(history, command);
    time_t start_time = time(NULL);

    char *commands[100];
    int i = 0;
    commands[i] = strtok(command, "|");
    while (commands[i] != NULL && i < 99) {
        i++;
        commands[i] = strtok(NULL, "|");
    }

    int num_commands = i;
    int pipefds[2 * (num_commands - 1)];

    for (i = 0; i < (num_commands - 1); i++) {
        if (pipe(pipefds + i * 2) == -1) {
            perror("pipe failed");
            return;
        }
    }

    int j = 0;
    for (i = 0; i < num_commands; i++) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork failed");
            return;
        } else if (pid == 0) {
            // Child process
            if (i < num_commands - 1) {
                if (dup2(pipefds[j + 1], STDOUT_FILENO) == -1) {
                    perror("dup2 failed");
                    exit(EXIT_FAILURE);
                }
            }
            if (i > 0) {
                if (dup2(pipefds[j - 2], STDIN_FILENO) == -1) {
                    perror("dup2 failed");
                    exit(EXIT_FAILURE);
                }
            }

            for (int k = 0; k < 2 * (num_commands - 1); k++) {
                close(pipefds[k]);
            }

            char *args[100];
            parse_command(commands[i], args);
            if (execvp(args[0], args) == -1) {
                perror("execvp failed");
                exit(EXIT_FAILURE);
            }
        }
        j += 2;
    }

    // Parent process
    for (i = 0; i < 2 * (num_commands - 1); i++) {
        close(pipefds[i]);
    }

    for (i = 0; i < num_commands; i++) {
        wait(NULL);
    }

    // Record execution time and duration in history2
    time_t end_time = time(NULL);
    if (history2->count < 20) {
        history2->pids[history2->count] = getpid(); // Parent process ID
        history2->execution_time[history2->count] = start_time;
        history2->execution_duration[history2->count] = difftime(end_time, start_time);
        history2->count++;
    }
}

// Function to execute a shell script
void execute_bash(char *script_name, struct History *history, struct History2 *history2) {
    FILE *script_file = fopen(script_name, "r");
    if (script_file) {
        char script_command[1200];

        while (fgets(script_command, sizeof(script_command), script_file) != NULL) {
            // Remove the trailing newline character
            remove_newline(script_command);

            // Execute the command from the script file
            char *result = execute_command(script_command, history, history2);
            printf("%s", result);
            free(result);
        }

        fclose(script_file);
    } else {
        perror("Error: Unable to open script file");
    }
}

int main(void) {
    char command[1200];

    // Initialize the command history
    struct History history = { .count = 0 };

    // Initialize the History2
    struct History2 history2 = { .count = 0 };

    do {
        printf("SimpleShell> ");
        if (fgets(command, sizeof(command), stdin) == NULL) {
            if (feof(stdin)) {
                printf("\nEnd of file reached. Exiting SimpleShell.\n");
            } else {
                perror("Error reading input");
            }
            break;
        }
        remove_newline(command);

        if (strlen(command) == 0) {
            continue;  // Skip empty commands
        }

        if (strcmp(command, "exit") == 0) {
            // Display the History2 (PID, execution time, duration)
            for (int i = 0; i < history2.count; i++) {
                printf("\nCommand %d:\n", i + 1);
                printf("%s\n", history.commands[i]);
                printf("PID: %d\n", history2.pids[i]);
                printf("Execution Time: %s", asctime(localtime(&history2.execution_time[i])));
                printf("Execution Duration: %.2f seconds\n", history2.execution_duration[i]);
            }
            break; // Exit the shell
        } else if (strstr(command, ".sh") != NULL && strstr(command, "./")) {
            // The command contains ".sh", indicating a script file
            execute_bash(command, &history, &history2);
        } else if (strcmp(command, "history") == 0) {
            // Display the command history
            printf("Command History:\n");
            for (int i = 0; i < history.count; i++) {
                printf("%d: %s\n", i + 1, history.commands[i]);
            }
        } else if (strncmp(command, "./", 2) == 0) {
            // Handle local executable
            char *result = execute_command(command, &history, &history2);
            printf("%s", result);
            free(result);
        } else {
            // Check for invalid characters in the command
            if (!is_valid_command(command)) {
                printf("Invalid command: commands should not include semicolons or ampersands.\n");
            } else if (strchr(command, '|') != NULL) {
                // Execute piped commands
                execute_piped_commands(command, &history, &history2);
            } else {
                // Execute the command and get the result
                char *result = execute_command(command, &history, &history2);
                printf("%s", result);
                free(result);
            }
        }
    } while (1);

    return 0;
}
