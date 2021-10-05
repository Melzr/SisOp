#include "builtin.h"

// returns true if the 'exit' call
// should be performed
//
// (It must not be called from here)
int
exit_shell(char *cmd)
{
	return (strcmp(cmd, "exit") == 0);
}

// returns true if "chdir" was performed
//  this means that if 'cmd' contains:
// 	1. $ cd directory (change to 'directory')
// 	2. $ cd (change to $HOME)
//  it has to be executed and then return true
//
//  Remember to update the 'prompt' with the
//  	new directory.
//
// Examples:
//  1. cmd = ['c','d', ' ', '/', 'b', 'i', 'n', '\0']
//  2. cmd = ['c','d', '\0']
int
cd(char *cmd)
{
	char *dir;

	if (strcmp(cmd, "cd") == 0 || strcmp(cmd, "cd ") == 0) {
		dir = getenv("HOME");
	} else if (strncmp(cmd, "cd ", 3) == 0) {
		dir = cmd+3;
	} else 
		return 0; 

	if (chdir(dir) < 0) {
		perror("Error: cannot cd");
		return 0;
	} else {
		char *buf = NULL;
		snprintf(promt, sizeof promt, "(%s)", getcwd(buf, 0));
		free(buf);		//// lo debe imprimir desde HOME?
	}																// falta liberarlo

	return 1;
}

// returns true if 'pwd' was invoked
// in the command line
//
// (It has to be executed here and then
// 	return true)
int
pwd(char *cmd)
{
	if (strcmp(cmd, "pwd") != 0)
		return 0;

	char* buf = NULL;
	printf("%s\n", getcwd(buf, 0)); //leak
	free(buf);

	return 1;
}
