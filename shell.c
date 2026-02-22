#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

extern char **environ;

char *trim_spaces(char *str)
{
    char *end;
    while (*str == ' ' || *str == '\t')
        str++;
    if (*str == '\0')
        return str;
    end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t'))
        *end-- = '\0';
    return str;
}

int main(void)
{
    char *line = NULL;
    size_t len = 0;
    ssize_t read_len;
    pid_t pid;
    int interactive;
    char *argv[2];
    argv[1] = NULL;

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

        line = trim_spaces(line);
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
            argv[0] = line;
            if (execve(line, argv, environ) == -1)
            {
                write(2, "./simple_shell: No such file or directory\n", 43);
                exit(1);
            }
            exit(0);
        }
        else
            wait(NULL);
    	free(line);
    }
    return 0;
}
