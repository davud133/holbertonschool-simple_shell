#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

/* Maximum command line length */
#define MAX_LINE 1024
#define MAX_ARGS 64

extern char **environ;

/**
 * trim_spaces - remove leading/trailing whitespace from a string
 * @str: string to trim
 */
void trim_spaces(char *str)
{
    char *start = str;
    char *end;
    while (*start == ' ' || *start == '\t')
        start++;
    if (*start == '\0')
    {
        str[0] = '\0';
        return;
    }
    end = start + strlen(start) - 1;
    while (end > start && (*end == ' ' || *end == '\t'))
        *end-- = '\0';
    if (start != str)
        memmove(str, start, strlen(start) + 1);
}

/**
 * parse_args - split a line into arguments
 * @line: input line
 * @argv: output array of argument strings
 * Return: number of arguments
 */
int parse_args(char *line, char **argv)
{
    int argc = 0;
    char *token = strtok(line, " \t");
    while (token != NULL && argc < (MAX_ARGS - 1))
    {
        argv[argc++] = token;
        token = strtok(NULL, " \t");
    }
    argv[argc] = NULL;
    return argc;
}

/**
 * is_executable - check if a file exists and is executable
 * @path: file path
 * Return: 1 if executable, 0 otherwise
 */
int is_executable(char *path)
{
    return access(path, X_OK) == 0;
}

/**
 * find_command_in_path - search PATH for command
 * @cmd: command name
 * Return: full path (malloc'd) or NULL if not found
 */
char *find_command_in_path(char *cmd)
{
    char *path_env = NULL;
    char *paths = NULL;
    char *dir = NULL;
    char *full_path = NULL;
    int i;

    /* Look for PATH in environ manually (no getenv) */
    for (i = 0; environ[i] != NULL; i++)
    {
        if (strncmp(environ[i], "PATH=", 5) == 0)
        {
            path_env = environ[i] + 5;
            break;
        }
    }

    /* If PATH is empty, return NULL */
    if (!path_env || path_env[0] == '\0')
        return NULL;

    /* Duplicate PATH to modify it */
    paths = strdup(path_env);
    if (!paths)
        return NULL;

    dir = strtok(paths, ":");
    while (dir != NULL)
    {
        full_path = malloc(strlen(dir) + 1 + strlen(cmd) + 1);
        if (!full_path)
        {
            free(paths);
            return NULL;
        }
        strcpy(full_path, dir);
        strcat(full_path, "/");
        strcat(full_path, cmd);
        if (is_executable(full_path))
        {
            free(paths);
            return full_path; /* Found executable */
        }
        free(full_path);
        full_path = NULL;
        dir = strtok(NULL, ":");
    }

    free(paths);
    return NULL; /* Not found */
}

/**
 * main - simple shell
 * Return: 0 on exit
 */
int main(void)
{
    char *line = NULL;
    size_t len = 0;
    ssize_t read_len;
    char *argv[MAX_ARGS];
    pid_t pid;
    int interactive;
    int status = 0;
    char *cmd_path;

    while (1)
    {
        interactive = isatty(STDIN_FILENO);
        if (interactive)
            write(1, "$:", 2);

        read_len = getline(&line, &len, stdin);
        if (read_len == -1)
        {
            if (interactive)
                write(1, "\n", 1);
            break;
        }

        if (line[read_len - 1] == '\n')
            line[read_len - 1] = '\0';

        trim_spaces(line);
        if (line[0] == '\0')
            continue;

        parse_args(line, argv);

        /* Check if command exists */
        if (strchr(argv[0], '/'))
        {
            cmd_path = argv[0];
            if (!is_executable(cmd_path))
            {
                write(2, "./hsh: 1: ", 11);
                write(2, argv[0], strlen(argv[0]));
                write(2, ": not found\n", 12);
                status = 127;
                continue;
            }
        }
        else
        {
            cmd_path = find_command_in_path(argv[0]);
            if (!cmd_path)
            {
                write(2, "./hsh: 1: ", 11);
                write(2, argv[0], strlen(argv[0]));
                write(2, ": not found\n", 12);
                status = 127;
                continue;
            }
        }

        pid = fork();
        if (pid < 0)
        {
            perror("fork");
            free(line);
            exit(1);
        }
        else if (pid == 0)
        {
            execve(cmd_path, argv, environ);
            write(2, "./hsh: 1: ", 11);
            write(2, argv[0], strlen(argv[0]));
            write(2, ": not found\n", 12);
            exit(127);
        }
        else
            wait(&status);

        if (!strchr(argv[0], '/'))
            free(cmd_path);
    }

    free(line);
    return status;
}
