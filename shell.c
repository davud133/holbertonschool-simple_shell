/**
 * simple_shell.c - Minimal shell that handles PATH without getenv()
 *
 * Description: Reads commands from stdin, resolves PATH from environ,
 * and executes them. Fork is only called if the command exists.
 *
 * Usage: ./simple_shell
 *
 * Exit Status:
 *   0   Success
 *   127 Command not found
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

extern char **environ;
#define MAX_ARGS 64

/**
 * trim_spaces - Remove leading and trailing whitespace
 * @str: String to trim
 */
void trim_spaces(char *str)
{
    char *start;
    char *end;
    size_t len;

    start = str;
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
    {
        len = strlen(start);
        memmove(str, start, len + 1);
    }
}

/**
 * find_in_environ - Get PATH from environ manually
 * Return: pointer to PATH string, or NULL if not found
 */
char *find_in_environ(void)
{
    int i;
    char *s;

    for (i = 0; environ[i]; i++)
    {
        s = environ[i];
        if (s[0] == 'P' && s[1] == 'A' && s[2] == 'T' &&
            s[3] == 'H' && s[4] == '=')
            return s + 5; /* skip "PATH=" */
    }
    return NULL;
}

/**
 * get_command_path - Resolve command using PATH
 * @cmd: command name
 * Return: full path if exists, else NULL
 */
char *get_command_path(char *cmd)
{
    char *path_env;
    char *paths;
    char *token;
    char *cmd_path;
    size_t cmd_len;
    size_t path_len;

    if (strchr(cmd, '/'))
    {
        if (access(cmd, X_OK) == 0)
            return cmd;
        return NULL;
    }

    path_env = find_in_environ();
    if (!path_env || path_env[0] == '\0')
        return NULL;

    paths = strdup(path_env);
    if (!paths)
        return NULL;

    token = strtok(paths, ":");
    while (token)
    {
        cmd_len = strlen(cmd);
        path_len = strlen(token);
        cmd_path = malloc(cmd_len + path_len + 2);
        if (!cmd_path)
        {
            free(paths);
            return NULL;
        }

        strcpy(cmd_path, token);
        strcat(cmd_path, "/");
        strcat(cmd_path, cmd);

        if (access(cmd_path, X_OK) == 0)
        {
            free(paths);
            return cmd_path;
        }

        free(cmd_path);
        token = strtok(NULL, ":");
    }

    free(paths);
    return NULL;
}

/**
 * main - entry point
 */
int main(void)
{
    char *line = NULL;
    size_t len = 0;
    ssize_t read_len;
    char *argv[MAX_ARGS];
    int i;
    pid_t pid;
    int interactive;
    int status;

    while (1)
    {
        interactive = isatty(STDIN_FILENO);
        if (interactive)
            write(1, ":) ", 3);

        read_len = getline(&line, &len, stdin);
        if (read_len == -1)
        {
            if (interactive)
                write(1, "\n", 1);
            free(line);
            exit(0);
        }

        if (line[read_len - 1] == '\n')
            line[read_len - 1] = '\0';

        trim_spaces(line);
        if (line[0] == '\0')
            continue;

        /* Tokenize */
        i = 0;
        argv[i] = strtok(line, " ");
        while (argv[i] && i < MAX_ARGS - 1)
        {
            i++;
            argv[i] = strtok(NULL, " ");
        }

        /* Resolve command path */
        {
            char *cmd_path;
            cmd_path = get_command_path(argv[0]);
            if (!cmd_path)
            {
                write(2, "./hsh: 1: ", 11);
                write(2, argv[0], strlen(argv[0]));
                write(2, ": not found\n", 12);
                continue;
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
                perror("execve");
                exit(127);
            }
            else
            {
                wait(&status);
            }

            if (cmd_path != argv[0])
                free(cmd_path);
        }
    }

    free(line);
    return 0;
}
