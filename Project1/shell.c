#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <regex.h>

#define MAX_COMMAND_LINE_LEN 1024
#define MAX_COMMAND_LINE_ARGS 128
#define TIMEOUT_SECONDS 10

char prompt[MAX_COMMAND_LINE_LEN];
char delimiters[] = " \t\r\n";
extern char **environ;

// Global variable to store the PID of the foreground process
pid_t foreground_pid = -1;

// Intercepts the SIGINT signal (triggered by pressing Ctrl+C) 
// to prevent the shell from terminating, allowing it to continue running
void sigint_handler(int signo) {
    // Do nothing; just return to the prompt
}

// Handles the SIGALRM signal by terminating the foreground process
// (if one is running) that exceeds a specified timeout, prints a 
// message about the termination, and resets the foreground process ID.
void sigalrm_handler(int signo) {
    if (foreground_pid > 0) {
        // Kill the foreground process if it exceeds the timeout
        kill(foreground_pid, SIGKILL);
        printf("\nProcess %d terminated due to timeout.\n", foreground_pid);
        foreground_pid = -1; // Reset the foreground PID
    }
}

// Retrieves current working directory and updates shell prompt to display it followed by a ">"
void update_prompt() {
    char cwd[MAX_COMMAND_LINE_LEN];
    getcwd(cwd, sizeof(cwd));
    snprintf(prompt, sizeof(prompt), "%s> ", cwd);
}

// Takes in specified path and changes current working directory
// to that path. Prints error message if no path or if change fails
void cd(char *path) {
    if (path) {
        if (chdir(path) != 0) {
            perror("cd failed");
        }
    } else {
        fprintf(stderr, "cd: missing argument\n");
    }
}

// Retrieves and prints the current working output
void pwd() {
    char cwd[MAX_COMMAND_LINE_LEN];
    getcwd(cwd, sizeof(cwd));
    printf("%s\n", cwd);
}

// Takes in arguments and prints them out. It handles environment
// variables prefixed with "$" by retrieving values with getenv. 
// Also ensures proper spacing between items
void echo(char **arguments) {
    int i;
    for (i = 1; arguments[i] != NULL; i++) {
        // Check if the current argument is an environment variable
        if (arguments[i][0] == '$') {
            char *var_value = getenv(arguments[i] + 1); // Get the variable's value
            if (var_value) {
                printf("%s ", var_value);
            }
            continue; // Skip printing the environment variable name
        }
        printf("%s", arguments[i]);

        // Print a space only if there's another argument
        if (arguments[i + 1] != NULL) {
            printf(" ");
        }
    }
    printf("\n"); // Newline at the end
}

// Takes in environment variable and prints out the value.
// If no environment variable given, prints all environment 
// variables currently set in environment
void env(char **arguments) {
    if (arguments[1] != NULL) {
        char *value = getenv(arguments[1]);
        if (value) {
            printf("%s\n", value);
        } else {
            fprintf(stderr, "env: %s: No such variable\n", arguments[1]);
        }
    } else {
        char **env;
        for (env = environ; *env != NULL; env++) {
            printf("%s\n", *env);
        }
    }
}

// Sets environment variable using format NAME=VALUE.
// Extracts variable name and value from input string by
// splitting on "=". If incorrect format, print error message
void setenv_command(char *name_value) {
    char *equal_sign = strchr(name_value, '=');
    if (equal_sign) {
        *equal_sign = '\0';
        char *name = name_value;
        char *value = equal_sign + 1;
        setenv(name, value, 1);
    } else {
        fprintf(stderr, "setenv: missing '='\n");
    }
}

// Splits input command line string into individual arguments based on 
// specified delimiters and stores them in an array
// Returns the total number of arguments passed
int tokenize_commandLine(char *command_line, char **arguments) {
    int argc = 0;
    char *token = strtok(command_line, delimiters);

    while (token != NULL && argc < MAX_COMMAND_LINE_ARGS - 1) {
        arguments[argc++] = token;
        token = strtok(NULL, delimiters);
    }
    arguments[argc] = NULL;
    return argc;
}

// Checks if the last argument in command line is "&" to determine
// if the command should be run in the background. It removes
// the "&" from the arguments array and return true if present, false if not
bool is_background_command(char **arguments, int argc) {
    if (argc > 0 && strcmp(arguments[argc - 1], "&") == 0) {
        arguments[--argc] = NULL;
        return true;
    }
    return false;
}

// Redirects standard input and output file descriptors if 
// necessary then executes the command specified
// A little redundant since I already defined most commands, but covers gaps
void execute_command(char **arguments, int input_fd, int output_fd) {
    // Redirect input and output if necessary
    if (input_fd != 0) {
        dup2(input_fd, STDIN_FILENO);
        close(input_fd);
    }
    if (output_fd != 1) {
        dup2(output_fd, STDOUT_FILENO);
        close(output_fd);
    }
    execvp(arguments[0], arguments);
    perror("exec failed");
    exit(EXIT_FAILURE); // Exit if exec fails
}

// Takes an array of arguments and two pointers to manage input and output
// redirection by parsing arguments for redirection operators ("<",">")
// Open specified files, updates file descriptors and NULL terminates the arguments
void handle_redirection(char **arguments, int *input_fd, int *output_fd) {
    int i;
    for (i = 0; arguments[i] != NULL; i++) {
        if (strcmp(arguments[i], ">") == 0) {
            *output_fd = open(arguments[i + 1], O_CREAT | O_WRONLY | O_TRUNC, 0644);
            if (*output_fd < 0) {
                perror("open output file failed");
                exit(EXIT_FAILURE);
            }
            arguments[i] = NULL; // Null terminate the arguments
            break; // Break after handling output redirection
        } else if (strcmp(arguments[i], "<") == 0) {
            *input_fd = open(arguments[i + 1], O_RDONLY);
            if (*input_fd < 0) {
                perror("open input file failed");
                exit(EXIT_FAILURE);
            }
            arguments[i] = NULL; // Null terminate the arguments
            break; // Break after handling input redirection
        }
    }
}

// Manages the execution of two commands connected by pipe. 
// Takes in array of arguments and pipe_index to identify location of the pipe operator 
// Creates a pipe, forks 2 child process to execute left and right commands while 
// redirecting their standard input and output to the pipe. Waits for both child processes to complete.
void handle_pipe(char **arguments, int pipe_index) {
    int pipe_fd[2];
    pid_t pid1, pid2;
    
    // Create a pipe
    if (pipe(pipe_fd) == -1) {
        perror("pipe failed");
        exit(EXIT_FAILURE);
    }

    // Fork first child for left command
    pid1 = fork();
    if (pid1 < 0) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    } else if (pid1 == 0) { // Child process for left command
        dup2(pipe_fd[1], STDOUT_FILENO); // Redirect stdout to pipe write end
        close(pipe_fd[0]); // Close unused read end
        close(pipe_fd[1]); // Close the write end after duplicating
        execvp(arguments[0], arguments); // Execute the left command
        perror("exec failed for left command");
        exit(EXIT_FAILURE);
    }

    // Fork second child for right command
    pid2 = fork();
    if (pid2 < 0) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    } else if (pid2 == 0) { // Child process for right command
        dup2(pipe_fd[0], STDIN_FILENO); // Redirect stdin to pipe read end
        close(pipe_fd[1]); // Close unused write end
        execvp(arguments[pipe_index + 1], &arguments[pipe_index + 1]); // Execute the right command
        perror("exec failed for right command");
        exit(EXIT_FAILURE);
    }

    // Close pipe in parent
    close(pipe_fd[0]);
    close(pipe_fd[1]);

    // Wait for both child processes
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
}

int main() {
    char command_line[MAX_COMMAND_LINE_LEN];
    char *arguments[MAX_COMMAND_LINE_ARGS];

    // Set up the signal handlers
    signal(SIGINT, sigint_handler);
    signal(SIGALRM, sigalrm_handler);

    while (true) {
        do {
            update_prompt();
            printf("%s", prompt);
            fflush(stdout);

            if ((fgets(command_line, MAX_COMMAND_LINE_LEN, stdin) == NULL) && ferror(stdin)) {
                fprintf(stderr, "fgets error");
                exit(0);
            }
        } while (command_line[0] == 0x0A); // while just ENTER pressed

        if (feof(stdin)) {
            printf("\n");
            fflush(stdout);
            return 0;
        }

        int argc = tokenize_commandLine(command_line, arguments);

        if (argc > 0) {
            if (strcmp(arguments[0], "exit") == 0) {
                exit(0);
            } else if (strcmp(arguments[0], "cd") == 0) {
                cd(arguments[1]);
                continue;
            } else if (strcmp(arguments[0], "pwd") == 0) {
                pwd();
                continue;
            } else if (strcmp(arguments[0], "echo") == 0) {
                echo(arguments);
                continue;
            } else if (strcmp(arguments[0], "env") == 0) {
                env(arguments);
                continue;
            } else if (strcmp(arguments[0], "setenv") == 0) {
                setenv_command(arguments[1]);
                continue;
            }
        }

        bool background = is_background_command(arguments, argc);

        // Check for piping
        int pipe_index = -1;
        int i;
        for (i = 0; i < argc; i++) {
            if (arguments[i] != NULL && strcmp(arguments[i], "|") == 0) {
                pipe_index = i;
                break;
            }
        }

        if (pipe_index != -1) {
            // We have a pipe
            arguments[pipe_index] = NULL; // Null terminate the left command
            handle_pipe(arguments, pipe_index);
            continue;
        }

        // Handle redirection
        int input_fd = 0;  // Default to stdin
        int output_fd = 1; // Default to stdout
        handle_redirection(arguments, &input_fd, &output_fd);

        // Fork a child process to execute the command
        pid_t pid = fork();
        if (pid < 0) {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // Child process
            execute_command(arguments, input_fd, output_fd);
        } else {
            // Parent process
            if (!background) {
                // Set the foreground process ID
                foreground_pid = pid;
                alarm(TIMEOUT_SECONDS); // Set a timer for 10 seconds

                int status;
                waitpid(pid, &status, 0); // Wait for the child process to finish

                // Cancel the alarm if the process completed in time
                alarm(0);

                // Reset the foreground PID
                foreground_pid = -1;

                if (WIFEXITED(status)) {
                    printf("Child exited with status %d\n", WEXITSTATUS(status));
                } else {
                    printf("Child terminated abnormally.\n");
                }
            } else {
                printf("Started background process with PID %d\n", pid);
            }
        }

        // Close file descriptors if they were opened
        if (input_fd != 0) close(input_fd);
        if (output_fd != 1) close(output_fd);
    }
    return -1; // This should never be reached.
}
