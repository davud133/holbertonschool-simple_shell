#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
extern char **environ;
int MAX_LINE = 1024;
int main(void)
{
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	pid_t pid;
	int status;
	int interactive = 0;
	char *argv[2];
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
		else if (line[0] == '\0')
		{
			continue;
		}
		else
		{
			if (line[read - 1] == '\n')
				line[read - 1] = '\0';
		}
		pid = fork();
		if (pid < 0)
			exit(1);
		else if (pid == 0)
		{
			argv[0] = line;
			if (execve(line, argv, environ) == -1)
			{
				write(2, "./simple_shell: No such file or directory\n", 43);
				exit(1);
			}
		}
		else
			wait(NULL);
	}
}
			

