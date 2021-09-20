#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

void primo_iteracion(int fd_izq[2]) {

	close(fd_izq[1]);
	int p = 0;

	if (read(fd_izq[0], &p, sizeof(p)) <= 0) {
		perror("Error en read");
		_exit(-1);
	}

	printf("primo %i\n", p);

	int n = 0;
	int lectura = read(fd_izq[0], &n, sizeof(n));
	if(lectura == 0) {
		close(fd_izq[0]);
		_exit(0);
	} else if (lectura < 0) {
		perror("Error en read");
		_exit(-1);
	}

	int fd_der[2];
	if ( pipe(fd_der) < 0 ){
		perror("Error en pipe");
		_exit(-1);
	}

	int f = fork();
	if (f < 0) {
		perror("Error en el fork");
		_exit(-1);
	}

	if (f > 0) {

		close(fd_der[0]);

		while( lectura != 0 ) {

			if (n%p != 0) {
				if (write(fd_der[1], &n, sizeof(n)) < 0) {
					perror("Error en write");
					_exit(-1);
				}	
			}

			lectura = read(fd_izq[0], &n, sizeof(n));
			if(lectura < 0) {
				perror("Error en read");
				_exit(-1);
			}
		}

		close(fd_izq[0]);
		close(fd_der[1]);
		int wstatus;
		int ret = wait(&wstatus);
		_exit(0);

	} else {
		close(fd_izq[0]);
		close(fd_der[1]);
		primo_iteracion(fd_der);
	}

}


int
main(int argc, char *argv[])
{
	if (argc != 2) {
		printf("Error: debe ingresar un unico numero como parametro\n");
		return(0);
	}

	int n = atoi(argv[1]);

	if (n < 2) {
		printf("Error: el numero ingresado debe ser mayor o igual a 2\n");
		return(0);
	}

	int numeros[n-1];

	for (int i = 2; i <= n; i++) {
		numeros[i-2]=i;
	}

	int fd[2];

	if (pipe(fd) < 0) {
		perror("Error en pipe");
		_exit(-1);
	}

	int i = fork();

	if (i < 0) {
		perror("Error en el fork");
		_exit(-1);
	}

	if (i > 0) {

		close(fd[0]);

		if (write(fd[1], numeros, sizeof(numeros)) < 0) {
			perror("Error en write");
			_exit(-1);
		}

		close(fd[1]);

		int wstatus;
		int ret = wait(&wstatus);		
		
		_exit(0); // debo agarra el codigo de error?

	} else { 

		primo_iteracion(fd);

	}

	return 0;
}