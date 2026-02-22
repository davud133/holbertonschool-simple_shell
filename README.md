# Simple Shell

Simple Shell is a basic UNIX command-line interpreter written in C.  
It replicates some core functionalities of the standard shell (/bin/sh).

## 📌 Description

This shell program:
- Displays a prompt
- Reads user input
- Parses commands
- Executes programs using fork and execve
- Handles built-in commands
- Supports PATH environment variable

The project demonstrates understanding of:
- Process creation (fork)
- Program execution (execve)
- Process control (wait)
- Environment variables
- Memory management
- String tokenization

---

## ⚙️ Compilation

Use GCC with the following flags:

```bash
gcc -Wall -Werror -Wextra -pedantic -std=gnu89 *.c -o hsh
