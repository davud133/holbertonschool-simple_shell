#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

extern char **environ;

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

char *get_command_path(char *cmd)
{
    char *path_env;
    char *paths;
    char *dir;
    static char full_path[1024];

    if (cmd[0] == '/' || cmd[0] == '.')
        return cmd;

    path_env = getenv("PATH");
    if (!path_env)
        return cmd;

    paths = strdup(path_env);
    dir = strtok(paths, ":");

    while (dir)
    {
        snprintf(full_path, sizeof(full_path), "%s/%s", dir, cmd);
        if (access(full_path, X_OK) == 0)
        {
            free(paths);
            return full_path;
        }
        dir = strtok(NULL, ":");
    }

    free(paths);
    return cmd;
}

int main(void)
{
    char *line = NULL;
    size_t len = 0;
    ssize_t read_len;
    pid_t pid;
    int interactive;
    char *argv[64];
    int argc;
    char *token;
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
            free(line);
            exit(0);
        }

        if (line[read_len - 1] == '\n')
            line[read_len - 1] = '\0';

        trim_spaces(line);
        if (line[0] == '\0')
            continue;

        pid = fork();
        if (pid < 0)
        {
            perror("fork");
            free(line);
            exit(1);
        }
        else if (pid == 0)
        {
            argc = 0;
            token = strtok(line, " ");
            while (token && argc < 63)
            {
                argv[argc++] = token;
                token = strtok(NULL, " ");
            }
            argv[argc] = NULL;

            cmd_path = get_command_path(argv[0]);
            if (execve(cmd_path, argv, environ) == -1)
            {
                write(2, "./simple_shell: No such file or directory\n", 43);
                exit(1);
            }
        }
        else
            wait(NULL);
    }

    free(line);
    return 0;
}
