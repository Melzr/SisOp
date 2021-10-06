#include "exec.h"

// sets "key" with the key part of "arg"
// and null-terminates it
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  key = "KEY"
//
static void
get_environ_key(char *arg, char *key)
{
	int i;
	for (i = 0; arg[i] != '='; i++)
		key[i] = arg[i];

	key[i] = END_STRING;
}

// sets "value" with the value part of "arg"
// and null-terminates it
// "idx" should be the index in "arg" where "=" char
// resides
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  value = "value"
//
static void
get_environ_value(char *arg, char *value, int idx)
{
	size_t i, j;
	for (i = (idx + 1), j = 0; i < strlen(arg); i++, j++)
		value[j] = arg[i];

	value[j] = END_STRING;
}

// sets the environment variables received
// in the command line
static void
set_environ_vars(char **eargv, int eargc)
{
	for (int i = 0; i < eargc; i++) {
		int index = block_contains(eargv[i], '=');
		size_t n = strlen(eargv[i]);
		if (index > 1) {
			char key[index + 1];
			char value[n - index];
			get_environ_key(eargv[i], key);
			get_environ_value(eargv[i], value, index);
			if (setenv(key, value, 1) < 0)
				fprintf_debug(stderr, "Error en setenv\n");
		}
	}
}

// opens the file in which the stdin/stdout/stderr
// flow will be redirected, and returns
// the file descriptor
//
// Find out what permissions it needs.
// Does it have to be closed after the execve(2) call?
static int
open_redir_fd(char *file, int flags)
{
	if (flags & O_CREAT)
		return open(file, flags, S_IWUSR | S_IRUSR);

	return open(file, flags);
}

// Sets file as fd
// Returns -1 on error
static int
set_fd(char *file, int flags, int fd)
{
	int new_fd = open_redir_fd(file, flags);
	if (dup2(new_fd, fd) < 0) {
		fprintf_debug(stderr, "Error al redirigir %s\n", file);
		close(new_fd);
		return -1;
	}
	close(new_fd);
	return 0;
}

// Sets file descriptors 0,1,2
// Returns -1 on error
static int
set_redir_fds(struct execcmd *cmd)
{
	int rdwr_flags = (O_CREAT | O_TRUNC | O_RDWR | O_CLOEXEC);
	int rd_flags = (O_CLOEXEC | O_RDONLY);

	if (strlen(cmd->out_file) > 0)
		if (set_fd(cmd->out_file, rdwr_flags, 1) < 0)
			return -1;

	if (strlen(cmd->in_file) > 0) {
		if (set_fd(cmd->in_file, rd_flags, 0) < 0)
			return -1;
	}

	if (strlen(cmd->err_file) > 0) {
		if (strcmp(cmd->err_file, "&1") == 0) {
			if (dup2(1, 2) < 0) {
				fprintf_debug(stderr,
				              "Error al redirigir stderr\n");
				return -1;
			}
		} else if (set_fd(cmd->err_file, rdwr_flags, 2) < 0)
			return -1;
	}

	return 0;
}

static void
pipe_coordinator(struct pipecmd *pipe_cmd, int *status)
{
	struct cmd *l = pipe_cmd->leftcmd;
	struct cmd *r = pipe_cmd->rightcmd;

	int fds[2];
	if (pipe(fds) < 0) {
		fprintf_debug(stderr, "Error en pipe\n");
		return;
	}

	int f_l = fork();
	if (f_l < 0) {
		fprintf_debug(stderr, "Error en fork\n");
		close(fds[0]);
		close(fds[1]);
		return;
	}

	if (f_l == 0) {  // left
		free_command(r);
		free(pipe_cmd);
		close(fds[0]);
		dup2(fds[1], 1);
		close(fds[1]);
		exec_cmd(l);
	}

	int f_r = fork();
	if (f_r < 0) {
		printf_debug("Error en fork");
		close(fds[0]);
		close(fds[1]);
		wait(NULL);
		return;
	}

	if ((f_l > 0) && (f_r == 0)) {  // right
		free_command(l);
		free(pipe_cmd);
		close(fds[1]);
		dup2(fds[0], 0);
		close(fds[0]);
		exec_cmd(r);
	}

	if ((f_l > 0) && (f_r > 0)) {  // coordinator
		close(fds[1]);
		close(fds[0]);
		waitpid(f_l, NULL, 0);
		waitpid(f_r, status, 0);
	}
}

// Returns -1 on malloc error
static int
argv_copy(char *argv[], int argc, char *argv_copy[])
{
	for (int i = 0; i < argc; i++) {
		argv_copy[i] = malloc(sizeof(char) * (strlen(argv[i]) + 1));
		if (argv_copy[i] == NULL)
			return -1;
		strcpy(argv_copy[i], argv[i]);
	}

	argv_copy[argc] = NULL;
	return 0;
}

static void
free_args(char *argv[], int argc)
{
	for (int i = 0; i < argc; i++) {
		free(argv[i]);
	}
}

static void
end_as_status(int status)
{
	if (WEXITSTATUS(status))
		_exit(WEXITSTATUS(status));
	else if (WIFSIGNALED(status))
		kill(getpid(), WTERMSIG(status));
	else
		_exit(status);
}

// executes a command - does not return
void
exec_cmd(struct cmd *cmd)
{
	// To be used in the different cases
	struct execcmd *e;
	struct backcmd *b;
	struct execcmd *r;
	struct pipecmd *p;
	int status = 0;

	switch (cmd->type) {
	case EXEC:

		e = (struct execcmd *) cmd;
		int f = fork();

		if (f < 0) {
			fprintf_debug(stderr, "Error en fork\n");
			free_command(cmd);
			_exit(-1);
		}

		if (f == 0) {
			int argc = e->argc;
			char *argv[argc + 1];

			if (argv_copy(e->argv, argc, argv) == -1) {
				free_command(cmd);
				fprintf_debug(stderr, "Error en malloc\n");
				_exit(-1);
			}

			set_environ_vars(e->eargv, e->eargc);
			free_command(cmd);
			execvp(argv[0], argv);
			fprintf_debug(stderr, "Error en execvp\n");
			free_args(argv, argc);
			_exit(-1);
		} else
			wait(&status);

		free_command(cmd);
		end_as_status(status);
		break;

	case BACK: {
		b = (struct backcmd *) cmd;
		struct cmd *c = b->c;
		free(b);
		exec_cmd(c);
		_exit(-1);
		break;
	}

	case REDIR: {
		r = (struct execcmd *) cmd;

		if (set_redir_fds(r) == 0) {
			cmd->type = EXEC;
			exec_cmd(cmd);
		} else {
			free_command(cmd);
			_exit(-1);
		}

		break;
	}

	case PIPE: {
		p = (struct pipecmd *) cmd;

		if ((p->leftcmd->type != EXEC) || (p->rightcmd->type == BACK)) {
			fprintf_debug(stderr, "No soportado\n");
			free_command(cmd);
			_exit(-1);
		}

		int f = fork();
		if (f < 0) {
			fprintf_debug(stderr, "Error en fork\n");
			free_command(cmd);
			_exit(-1);
		}

		if (f == 0) {
			pipe_coordinator(p, &status);
		} else
			wait(&status);

		free_command(cmd);
		end_as_status(status);
		break;
	}
	}
}
