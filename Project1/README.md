Zarif H. Cabrera

Overview: 

For this project, I made a simple command shell that mimics basic shell functionality. It allows users to execute commands interactively, manage their environment, and perform essential tasks such as navigating directories, displaying the current directory, and setting environment variables. My goal was to build an intuitive and efficient tool while keeping the implementation straightforward.

Design Choices

1. Architecture and Structure

I structured the command shell as a single C program, breaking down the main functionality into clear and distinct functions. This separation makes the code easier to read, understand, and maintain as I could identify and modify functions not working as intended. Here are some key components of my code:

- Command Handling: Functions like “cd”, “pwd”, “echo”, “env”, and “setenv” provide the core functionality for built-in commands.
- Command Parsing: The “tokenize_commandLine” function is responsible for breaking down user input into manageable arguments. This function simplifies the command processing logic.
- Process Management: The “execute_command” function is responsible for creating a new process to run the specified command, while “handle_pipe” and “handle_redirection” manage input/output redirection and piping between commands.

I chose this modular approach because it makes it easier to extend functionality in the future and keeps the code clean. I also don’t have to worry about a bunch of different files.

1. update_prompt
This function retrieves the current working directory and updates the prompt string to include the path followed by a > symbol. This design choice enhances usability by giving users clear context about their current location in the file system.

2. cd
The cd function allows users to change the current working directory, using chdir to perform the operation. It includes error handling to provide immediate feedback if the directory change fails, which improves user experience by ensuring they are informed of any issues.

3. pwd
This function prints the current directory to the standard output using getcwd. This satisfies the fundamental need for users to know their location in the file system.

4. echo
The echo function prints both string arguments and environment variables (prefixed by $). It retrieves variable values using getenv, providing users with dynamic output. Proper spacing between arguments enhances readability, demonstrating attention to user experience.

5. env
This function retrieves and displays the value of a specified environment variable or lists all variables if no argument is given. It offers flexibility for users to interact with their environment, and includes error handling for missing variables to ensure clarity.

6. setenv_command
The setenv_command function parses a name-value pair to set environment variables using setenv. Its straightforward design focuses on efficiency, ensuring users can easily manage environment settings without complex parsing.

7. tokenize_commandLine
Using strtok, this function splits user input into tokens based on delimiters. It populates an array with these tokens while ensuring proper null termination. This method efficiently prepares input for further processing, maintaining clarity and ease of use.

8. is_background_command
This function checks if the last argument is &, indicating a background command. If found, it adjusts the argument list accordingly. This simple check allows users to easily specify background tasks, contributing to a more streamlined execution process. Validated using sleep 5 &. I ran into issues after adding piping code due to strcmp trying to read NULL (no arguments), but I fixed by adding, arguments[i] != NULL, to conditional on line 265.

9. execute_command
The execute_command function handles command execution using execvp, with input and output redirection based on provided file descriptors. By centralizing command execution logic, it ensures versatility and provides robust error handling for execution failures.

10. handle_redirection
This function scans for input (<) and output (>) redirection symbols in command arguments, opening the specified files for reading or writing. By modifying the arguments array and ensuring proper null termination, it simplifies the management of stream redirection for subsequent command execution.

11. handle_pipe
The handle_pipe function creates a pipe and forks two child processes to set up communication between them. It effectively manages input/output redirection using the pipe, allowing for complex command sequences while maintaining resource efficiency by closing unused pipe ends. (This was the hardest to implement since grep was not working, validated my function using cat test.txt | sort, ls | wc -l, cat test.txt | uniq -c)

12. main
The main function orchestrates the interpreter's core logic, managing user input, command parsing, and function execution. By integrating signal handling and process management, it ensures a smooth and responsive user experience, allowing for effective interaction with the CLI. 

Documentation of Code

I aimed for clarity in the code by using clear function names, inline comments, and consistent formatting. Here are some key documentation aspects:

1. Function Documentation: Each function has a comment explaining its purpose, parameters, and expected behavior. This documentation helps anyone reviewing the code quickly understand what each part does without diving too deep into the implementation.

2. Error Handling: I made sure to include error handling for functions that interact with system calls (like `chdir`, `open`, and `execvp`). When a failure occurs, the program prints a descriptive error message to `stderr`, so users are aware of any issues that arise.

3. Constants and Macros: I defined constants like `MAX_COMMAND_LINE_LEN`, `MAX_COMMAND_LINE_ARGS`, and `TIMEOUT_SECONDS` at the top of the file. This centralizes configuration and makes it easy to adjust parameters as needed.

4. Global Variables: The global variable `foreground_pid` tracks the currently executing foreground process. While using a global variable can be a point of contention, I believe it’s justified here for managing process control and signaling effectively.

5. Signal Handlers: The signal handlers are documented to indicate their purpose, which clarifies how the program responds to user interrupts and timeouts.

