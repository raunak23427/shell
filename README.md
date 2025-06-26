# SimpleShell

SimpleShell is a basic Unix shell implementation in C, developed as part of CSE 231: Operating System Assignment 2.

## Features

- Basic command execution
- Command piping
- Background process execution
- Command history tracking
- Execution of shell scripts

## Setup and Installation

### Prerequisites
- GCC compiler
- GNU Make
- Git (for version control)

### Clone the Repository
```bash
git clone https://github.com/raunak23427/shell
cd SimpleShell
```

### Compile the Project
```bash
make
```
This will create an executable named `simple-shell`.

## Usage Manual

### Starting the Shell
To start SimpleShell, run:
```bash
./simple-shell
```

### Basic Command Execution
Simply type the command and press Enter:
```
SimpleShell> ls -l
```

### Piping Commands
Use the `|` symbol to pipe commands:
```
SimpleShell> cat file.txt | grep pattern
```

### Background Processes
Add `&` at the end of a command to run it in the background:
```
SimpleShell> long_running_command &
```

### Command History
To view command history, type:
```
SimpleShell> history
```

### Executing Shell Scripts
To run a shell script:
```
SimpleShell> ./myscript.sh
```

### Exiting the Shell
To exit SimpleShell, type:
```
SimpleShell> exit
```
This will also display the execution history with PIDs, execution times, and durations.

## Limitations
- Maximum 99 arguments per command
- History limited to 20 commands
- No support for shell built-ins (cd, export, etc.)
- Limited error handling for complex scenarios
- No support for input/output redirection

## Cleaning Up
To remove the compiled executable:
```bash
make clean
```

## Contributing
M Rayhankhan 2022269 	Raunak Kumar Giri 2023427
