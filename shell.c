#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

/**
 * split_line - split input line into arguments
 * @line: input line
 * Return: NULL-terminated array of strings
 */
char **split_line(char *line)
{
    int bufsize = 64, pos = 0;
    char **tokens, *token;

    tokens = malloc(sizeof(char *) * bufsize);
    if (!tokens)
    {
        fprintf(stderr, "Allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, " \t\r\n");
    while (token != NULL)
    {
        tokens[pos++] = token;

        if (pos >= bufsize)
        {
            bufsize += 64;
            tokens = realloc(tokens, sizeof(char *) * bufsize);
            if (!tokens)
            {
                fprintf(stderr, "Allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, " \t\r\n");
    }
    tokens[pos] = NULL;
    return tokens;
}

/**
 * command_exists - check if command exists (absolute or relative)
 * @cmd: command name
 * Return: full path or NULL
 */
char *command_exists(char *cmd)
{
    char *path_env, *path_dup, *dir, *fullpath;
    char *path_sep = ":";
    int len;

    /* Check if cmd contains '/' (absolute or relative) */
    if (strchr(cmd, '/') != NULL)
    {
        if (access(cmd, X_OK) == 0)
            return cmd;
        return NULL;
    }

    /* Use PATH from environment */
    path_env = getenv("PATH");
    if (!path_env || path_env[0] == '\0')
        return NULL;

    path_dup = strdup(path_env);
    if (!path_dup)
    {
        fprintf(stderr, "Allocation error\n");
        exit(EXIT_FAILURE);
    }

    dir = strtok(path_dup, path_sep);
    while (dir != NULL)
    {
        len = strlen(dir) + strlen(cmd) + 2;
        fullpath = malloc(len);
        if (!fullpath)
        {
            fprintf(stderr, "Allocation error\n");
            exit(EXIT_FAILURE);
        }
        snprintf(fullpath, len, "%s/%s", dir, cmd);
        if (access(fullpath, X_OK) == 0)
        {
            free(path_dup);
            return fullpath;
        }
        free(fullpath);
        dir = strtok(NULL, path_sep);
    }

    free(path_dup);
    return NULL;
}

/**
 * execute - execute command
 * @args: arguments array
 */
void execute(char **args)
{
    pid_t pid;
    int status;
    char *cmd_path;

    if (args[0] == NULL)
        return;

    cmd_path = command_exists(args[0]);
    if (!cmd_path)
    {
        fprintf(stderr, "./hsh: 1: %s: not found\n", args[0]);
        return;
    }

    pid = fork();
    if (pid == 0)
    {
        /* child process */
        execve(cmd_path, args, environ);
        /* if execve fails */
        fprintf(stderr, "./hsh: 1: %s: not found\n", args[0]);
        exit(127);
    }
    else if (pid > 0)
    {
        /* parent waits for child */
        wait(&status);
        (void)status; /* suppress unused warning */
    }
    else
    {
        perror("fork");
    }
}

int main(void)
{
    char *line = NULL;
    size_t len = 0;
    char **args;
    ssize_t read;

    while (1)
    {
        printf(":) ");
        fflush(stdout);

        read = getline(&line, &len, stdin);
        if (read == -1)
        {
            free(line);
            break;
        }

        args = split_line(line);
        execute(args);
        free(args);
    }

    return 0;
}
