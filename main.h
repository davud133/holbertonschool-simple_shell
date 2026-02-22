#ifndef MAINH
#define MAINH

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>

#define MAX_LEN 100
#define PROMPT ">> "
#define PATH "PATH="
extern char **environ;
void handle_command(char *u_command);
void print_env(void);
void setup_environment(char **path_env, char **path);
void execute_command(char **args, char *path);
void parse_command(char *u_command, char **args);
void handle_path(char **args, char **path, char **path_env, int *found);
void handle_command(char *u_command);
void process_commands(char *commands, char **commands_array);
void handle_commands_array(char **commands_array);

#endif
