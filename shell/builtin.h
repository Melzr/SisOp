#ifndef BUILTIN_H
#define BUILTIN_H

#include "defs.h"

extern char promt[PRMTLEN];
extern int status;

int cd(char *cmd);

int exit_shell(char *cmd);

int pwd(char *cmd);

#endif  // BUILTIN_H
