#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h>

extern char **environ;

#define MAX_LINE 1024

/**
 * trim_spaces - removes leading and trailing spaces from a string
 * @str: string to trim
 *
 * Return: pointer to trimmed string (same buffer)
 */
char *trim_spaces(char *str)
{
    char *end;

    // Trim leading spaces
    while (*str && isspace((unsigned char)*str))
        str++;

    if (*str == 0) // All spaces
        return str;

    // Trim trailing spaces
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end))
        *end-- = '\0';

    return str;
}

int main(void)
{
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    pid_t pid;
    int interactive;
    char *argv[2]; // only argv[0] is needed
    argv[1] = NULL;

    while (1)
    {
        interactive = isatty(STDIN_FILENO);
        if (interactive)
            write(1, "$:", 2);

        read = getline(&line, &len, stdin);
        if (read == -1)
        {
            if (interactive)
                write(1, "\n", 1);
            free(line);
            exit(0);
        }

        // Remove newline and trim spaces
        if (line[read - 1] == '\n')
            line[read - 1] = '\0';
        line = trim_spaces(line);

        if (line[0] == '\0') // skip empty lines
            continue;

        pid = fork();
        if (pid < 0)
        {
            perror("fork");
            free(line);
            exit(1);
        }
        else if (pid == 0) // child
        {
            argv[0] = line;
            if (execve(line, argv, environ) == -1)
            {
                write(2, "./simple_shell: No such file or directory\n", 43);
                exit(1);
            }
            exit(0); // never reached if execve succeeds
        }
        else // parent
        {
            wait(NULL);
        }
    }

    free(line);
    return 0;
}
