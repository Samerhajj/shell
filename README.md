# Basic Shell with Redirection and Pipes Support

This project is a basic shell implementation in C that provides a command-line interface with support for input/output redirection and pipes. The shell allows users to interact with the operating system, execute commands, and manipulate their input/output streams efficiently.

## Key Features

- **Command Execution**: The shell supports executing external commands with arguments, allowing users to run programs and utilities.
- **Input/Output Redirection**: Users can redirect input from a file or command output to a file using the `<` and `>` symbols, respectively. This feature enables the shell to read input from files and save command output to files.
- **Pipes**: The shell supports pipe functionality using the `|` symbol, allowing users to chain commands together and redirect the output of one command as input to another. This enables the creation of command pipelines, enhancing the shell's versatility and power.
- **Interactive User Interface**: The shell provides a command prompt where users can enter commands and interact with the shell. It supports command history, allowing users to navigate and reuse previously executed commands.
- **Error Handling**: The shell includes proper error handling mechanisms to provide informative error messages and handle exceptional cases gracefully.
- **Signal Handling**: The shell handles signals such as Ctrl+C (SIGINT) and Ctrl+Z (SIGTSTP) to ensure proper termination and suspension of processes.
- **Basic Built-in Commands**: The shell implements a set of basic built-in commands, including changing directories (cd) and displaying the current working directory (pwd).

## Purpose

This project serves as an educational resource for understanding shell concepts, process management, and I/O redirection. It aims to provide a solid foundation for building more advanced shell functionalities and exploring the internals of operating systems.

Contributions, feedback, and enhancements to this basic shell implementation are welcome. Use it as a starting point for your own shell projects or as a learning tool to dive into the world of shells and system programming.

**Note**: This basic shell implementation focuses on core functionalities and does not include advanced features like environment variables, job control, or scripting capabilities.
