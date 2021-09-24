#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>


int
main(int argc, char *argv[])
{
	if (argc != 3) {
		printf("Error: se debe usar %s <archivo a copiar> <archivo "
		       "nuevo>\n",
		       argv[0]);
		exit(-1);
	}

	struct stat sb;

	if (lstat(argv[1], &sb) == -1) {
		perror("Error en lstat");
		exit(-1);
	}

	if (!S_ISREG(sb.st_mode)) {
		printf("Error: el archivo a copiar debe ser un archivo "
		       "regular\n");
		exit(-1);
	}

	int fd_fuente = open(argv[1], O_RDONLY);
	if (fd_fuente < 0) {
		perror("Error abriendo el archivo fuente");
		exit(-1);
	}

	int fd_destino = open(argv[2],
	                      O_RDWR | O_CREAT | O_EXCL,
	                      sb.st_mode);  // Falla si ya existe
	if (fd_destino < 0) {
		perror("Error creando el archivo destino");
		close(fd_fuente);
		exit(-1);
	}

	if (ftruncate(fd_destino, sb.st_size) < 0) {
		perror("Error en ftruncate");
		close(fd_destino);
		close(fd_fuente);
		exit(-1);
	}

	void *memoria_fuente =
	        mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd_fuente, 0);
	void *memoria_destino =
	        mmap(NULL, sb.st_size, PROT_WRITE, MAP_SHARED, fd_destino, 0);

	if ((memoria_destino == MAP_FAILED) || (memoria_fuente == MAP_FAILED)) {
		perror("Error en mmap");
		close(fd_fuente);
		close(fd_destino);
		exit(-1);
	}

	memcpy(memoria_destino, memoria_fuente, sb.st_size);

	close(fd_fuente);
	close(fd_destino);

	return 0;
}
