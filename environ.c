#include "main.h"
/**
 * print_env - func for printing rnv
*/
void print_env(void)
{
	char **env_ptr = environ;

	while (*env_ptr != NULL)
	{
		printf("%s\n", *env_ptr);
		env_ptr++;
	}
}
/**
 * setup_environment - funv for environment setup
 * @path_env: environment path
 * @path: path
*/
void setup_environment(char **path_env, char **path)
{
	int i;

	*path = malloc(MAX_LEN);
	if (!*path)
	{
		perror("Malloc failed");
		exit(EXIT_FAILURE);
	}
	memset(*path, 0, MAX_LEN);

	for (i = 0; environ[i] != NULL; i++)
		if (strncmp(environ[i], PATH, 5) == 0)
		{
			*path_env = strdup(environ[i] + 5);
			break;
		}
}
