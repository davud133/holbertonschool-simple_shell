#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

extern char **environ;

#define MAX_ARGS 64

/* Trim leading and trailing spaces/tabs */
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

/* Search command in PATH */
char *get_command_path(char *cmd)
{
    char *path_env;
    char *paths;
    char *token;
    char *cmd_path;
    size_t cmd_len;
    size_t path_len;

    if (strchr(cmd, '/')) /* If command contains /, try as is */
        return cmd;

    path_env = getenv("PATH");
    if (!path_env || path_env[0] == '\0') /* Empty PATH */
        return cmd;

    paths = strdup(path_env);
    if (!paths)
        return cmd;

    token = strtok(paths, ":");
    while (token)
    {
        cmd_len = strlen(cmd);
        path_len = strlen(token);
        cmd_path = malloc(cmd_len + path_len + 2);
        if (!cmd_path)
        {
            free(paths);
            return cmd;
        }
        strcpy(cmd_path, token);
        strcat(cmd_path, "/");
        strcat(cmd_path, cmd);

        if (access(cmd_path, X_OK) == 0)
        {
            free(paths);
            return cmd_path; /* Found executable */
        }

        free(cmd_path);
        token = strtok(NULL, ":");
    }

    free(paths);
    return cmd; /* Not found */
}

int main(void)
{
    char *line;
    size_t len;
    ssize_t read_len;
    pid_t pid;
    int interactive;
    char *argv[MAX_ARGS];
    int i;
    int status;
    int line_number = 1;

    line = NULL;
    len = 0;

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
            free(line);
            exit(0);
        }

        if (line[read_len - 1] == '\n')
            line[read_len - 1] = '\0';

        trim_spaces(line);
        if (line[0] == '\0')
        {
            line_number++;
            continue;
        }

        /* Split line into argv */
        i = 0;
        argv[i] = strtok(line, " ");
        while (argv[i] && i < MAX_ARGS - 1)
        {
            i++;
            argv[i] = strtok(NULL, " ");
        }

        pid = fork();
        if (pid < 0)
        {
            perror("fork");
            free(line);
            exit(1);
        }
        else if (pid == 0) /* Child */
        {
            char *cmd_path;

            cmd_path = get_command_path(argv[0]);
            execve(cmd_path, argv, environ);

            /* If execve fails, print exact expected message */
            write(2, "./hsh: ", 7);
            {
                char numbuf[12];
                int n = snprintf(numbuf, sizeof(numbuf), "%d", line_number);
                write(2, numbuf, n);
            }
            write(2, ": ", 2);
            write(2, argv[0], strlen(argv[0]));
            write(2, ": not found\n", 12);
            exit(127);
        }
        else /* Parent */
        {
            wait(&status);
        }

        line_number++;
    }

    free(line);
    return 0;
}
