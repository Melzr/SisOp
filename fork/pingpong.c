#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>

int
main(void)
{
	srandom(time(NULL));

	int fds_1[2];
	int fds_2[2];
	int pipe_1 = pipe(fds_1);
	int pipe_2 = pipe(fds_2);

	if (pipe_1 < 0 || pipe_2 < 0) {
		perror("Error en pipe");
		_exit(-1);
	}

	printf("Hola, soy PID %d:\n", getpid());
	printf("\t- primer pipe me devuelve: [%i, %i]\n", fds_1[0], fds_1[1]);
	printf("\t- segundo pipe me devuelve: [%i, %i]\n\n", fds_2[0], fds_2[1]);

	int i = fork();
	if (i < 0) {
		perror("Error en fork");
		_exit(-1);
	}

	if (i == 0) {
		close(fds_1[1]);
		close(fds_2[0]);

		int lectura = 0;

		if (read(fds_1[0], &lectura, sizeof(lectura)) < 0) {
			perror("Error en read");
			_exit(-1);
		}

		printf("Donde fork me devuelve %i:\n", i);
		printf("\t- getpid me devuelve: %d\n", getpid());
		printf("\t- getppid me devuelve: %d\n", getppid());
		printf("\t- recibo valor %i via fd=%i\n", lectura, fds_1[0]);
		printf("\t- reenvio valor en fd=%i y termino\n\n", fds_2[1]);

		close(fds_1[0]);

		if (write(fds_2[1], &lectura, sizeof(lectura)) < 0) {
			perror("Error en write");
			_exit(-1);
		}

		close(fds_2[1]);
		_exit(0);

	} else {
		close(fds_1[0]);
		close(fds_2[1]);

		int escritura = random();
		int lectura = 0;

		printf("Donde fork me devuelve %i:\n", i);
		printf("\t- getpid me devuelve: %d\n", getpid());
		printf("\t- getppid me devuelve: %d\n", getppid());
		printf("\t- random me devuelve: %i\n", escritura);

		if (write(fds_1[1], &escritura, sizeof(escritura)) < 0) {
			perror("Error en write");
			_exit(-1);
		}

		printf("\t- envio valor %i a traves de fd=%i\n\n",
		       escritura,
		       fds_1[1]);
		close(fds_1[1]);

		if (read(fds_2[0], &lectura, sizeof(lectura)) < 0) {
			perror("Error en read");
			_exit(-1);
		}

		printf("Hola, de nuevo PID %d:\n", getpid());
		printf("\t- recibi valor %i via fd=%i\n", lectura, fds_2[0]);
		close(fds_2[0]);
		wait(NULL);
		_exit(0);
	}

	return 0;
}
