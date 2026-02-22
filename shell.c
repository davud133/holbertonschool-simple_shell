#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

/**
 * trim_spaces - remove leading and trailing spaces
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
 * command_exists - check if a command exists in PATH or as relative/absolute
 * @cmd: command string
 * @full_path: buffer to store full path if exists
 * Return: 1 if command exists, 0 otherwise
 */
int command_exists(char *cmd, char *full_path)
{
    char *path_env = NULL;
    char *path_dup;
    char *token;
    char candidate[1024];
    int found = 0;
    int i;

    /* absolute or relative path */
    if (cmd[0] == '/' || cmd[0] == '.')
    {
        if (access(cmd, X_OK) == 0)
        {
            strcpy(full_path, cmd);
            return 1;
        }
        return 0;
    }

    /* manually get PATH from environ */
    {
        extern char **environ;
        for (i = 0; environ[i] != NULL; i++)
        {
            if (strncmp(environ[i], "PATH=", 5) == 0)
            {
                path_env = environ[i] + 5;
                break;
            }
        }
    }

    if (!path_env || path_env[0] == '\0')
        return 0; /* PATH empty, cannot find */

    path_dup = strdup(path_env);
    if (!path_dup)
        return 0;

    token = strtok(path_dup, ":");
    while (token)
    {
        strcpy(candidate, token);
        strcat(candidate, "/");
        strcat(candidate, cmd);
        if (access(candidate, X_OK) == 0)
        {
            strcpy(full_path, candidate);
            found = 1;
            break;
        }
        token = strtok(NULL, ":");
    }

    free(path_dup);
    return found;
}

/**
 * main - simple shell
 * Return: 0
 */
int main(void)
{
    char *line = NULL;
    size_t len = 0;
    ssize_t read_len;
    pid_t pid;
    char *argv[64];
    int i;
    char full_path[1024];

    while (1)
    {
        if (isatty(STDIN_FILENO))
            write(1, "$:", 2);

        read_len = getline(&line, &len, stdin);
        if (read_len == -1)
        {
            if (isatty(STDIN_FILENO))
                write(1, "\n", 1);
            free(line);
            exit(0);
        }

        if (line[read_len - 1] == '\n')
            line[read_len - 1] = '\0';

        trim_spaces(line);
        if (line[0] == '\0')
            continue;

        /* split command into argv */
        argv[0] = strtok(line, " \t");
        if (!argv[0])
            continue;
        for (i = 1; i < 64; i++)
        {
            argv[i] = strtok(NULL, " \t");
            if (!argv[i])
                break;
        }

        /* check command existence */
        if (!command_exists(argv[0], full_path))
        {
            write(2, "./hsh: 1: ", 11);
            write(2, argv[0], strlen(argv[0]));
            write(2, ": not found\n", 12);
            continue; /* do NOT fork */
        }

        pid = fork();
        if (pid < 0)
        {
            write(2, "Fork failed\n", 12);
            continue;
        }
        else if (pid == 0)
        {
            execve(full_path, argv, NULL);
            /* execve failed */
            write(2, "./hsh: 1: ", 11);
            write(2, argv[0], strlen(argv[0]));
            write(2, ": not found\n", 12);
            exit(127);
        }
        else
        {
            int status;
            wait(&status);
            /* propagate 127 if child exec failed */
            if (WIFEXITED(status) && WEXITSTATUS(status) == 127)
                ; /* nothing to do, child handled exit */
        }
    }

    free(line);
    return 0;
}
