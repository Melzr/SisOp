#include "printstatus.h"

// prints information of process' status
void
print_status_info(struct cmd *cmd)
{
	if (cmd->type == PIPE) {
		if (WIFEXITED(status))
			status = WEXITSTATUS(status);
		else if (WIFSIGNALED(status))
			status = -WTERMSIG(status);
		else if (WIFSTOPPED(status))
			status = -WSTOPSIG(status);
		return;
	}

	if (strlen(cmd->scmd) == 0)
		return;

	if (WIFEXITED(status)) {
#ifndef SHELL_NO_INTERACTIVE
		fprintf(stdout,
		        "%s	Program: [%s] exited, status: %d %s\n",
		        COLOR_BLUE,
		        cmd->scmd,
		        WEXITSTATUS(status),
		        COLOR_RESET);
#endif
		status = WEXITSTATUS(status);
	} else if (WIFSIGNALED(status)) {
#ifndef SHELL_NO_INTERACTIVE
		fprintf(stdout,
		        "%s	Program: [%s] killed, status: %d %s\n",
		        COLOR_BLUE,
		        cmd->scmd,
		        -WTERMSIG(status),
		        COLOR_RESET);
#endif
		status = -WTERMSIG(status);
	} else if (WIFSTOPPED(status)) {
#ifndef SHELL_NO_INTERACTIVE
		fprintf(stdout,
		        "%s	Program: [%s] stopped, status: %d %s\n",
		        COLOR_BLUE,
		        cmd->scmd,
		        -WSTOPSIG(status),
		        COLOR_RESET);
#endif
		status = -WSTOPSIG(status);
	}
}

// prints info when a background process is spawned
void
print_back_info(struct cmd *back)
{
#ifndef SHELL_NO_INTERACTIVE
	fprintf(stdout, "%s  [PID=%d] %s\n", COLOR_BLUE, back->pid, COLOR_RESET);
#endif
}
