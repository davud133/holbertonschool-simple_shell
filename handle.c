#include "main.h"
/**
 * handle_path - funv for handling path
 * @args: arguments to path
 * @path: path
 * @path_env: environment path
 * @found: int variable
*/
void handle_path(char **args, char **path, char **path_env, int *found)
{
	char *path_token = NULL;

	if (*path_env == NULL)
	{
		fprintf(stderr, "./hsh: 1: %s: not found\n", args[0]);
		free(*path_env);
		free(*path);
		exit(127);
	}
	path_token = strtok(*path_env, ":");

	while (path_token != NULL)
	{
		strcpy(*path, path_token);
		strcat(*path, "/");
		strcat(*path, args[0]);
		if (access(*path, X_OK) != -1)
		{
			*found = 1;
			break;
		}
		path_token = strtok(NULL, ":");
	}
	free(*path_env);
}
/**
 * handle_command - command handler
 * @u_command: command
*/
void handle_command(char *u_command)
{
	char *args[MAX_LEN], *path = NULL, *path_env = NULL;
	int found = 0;

	setup_environment(&path_env, &path);
	parse_command(u_command, args);

	if (args[0] == NULL)
	{
		free(path_env);
		free(path);
		return;
	}

	if (strchr(args[0], '/') != NULL)
	{
		if (access(args[0], X_OK) == -1)
		{
			fprintf(stderr, "./hsh: 1: %s: not found\n", args[0]);
			free(path);
			free(path_env);
			exit(127);
		}
		free(path);
		free(path_env);
		path = strdup(args[0]);
		found = 1;
	}
	else
		handle_path(args, &path, &path_env, &found);

	if (found == 0)
	{
		fprintf(stderr, "./hsh: 1: %s: not found\n", args[0]);
		free(path);
		exit(127);
	}

	execute_command(args, path);
	free(path);
}
