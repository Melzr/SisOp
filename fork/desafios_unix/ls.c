#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
#include <dirent.h>

#ifndef PATH_MAX
#define PATH_MAX 1000
#endif


/*
 * Imprime recursivamente a partir del directorio actual archivos y subdirectorios
 */
void recorrer_directorio(int fds);

/*
 * Dado el nombre de un archivo que se encuentra en el directorio actual
 * imprimira su tipo, permisos, usuario, nombre y a que archivo o directorio
 * apunta en caso de ser un enlace simbolico
 */
void mostrar_informacion(char* nombre);

/*
 * Dado un mode_t valido de un archivo completara las primeras 3 posiciones con
 * r w x si el usuario tiene permisos de lectura escritura y ejecucion 
 * respectivamente, las siguientes 3 posiciones si el grupo tiene esos permisos
 * y las ultimas 3 si otros tienen esos permisos
 */
void completar_permisos(char permisos[9], mode_t mode);


int
main()
{
	DIR *directorio = opendir(".");
	if (directorio == NULL) {
		perror("Error en opendir");
		exit(-1);
	}

	recorrer_directorio(dirfd(directorio));

	closedir(directorio);

	return 0;
}

void
recorrer_directorio(int fds)
{
	DIR *directorio = fdopendir(fds);
	if (directorio == NULL) {
		perror("Error en fdopendir");
		return;
	}

	printf("%-12s%-6s%s\n", "Permisos", "UID", "Nombre");

	struct dirent *entry = readdir(directorio);

	while (entry != NULL) {	
		mostrar_informacion(entry->d_name);
		entry = readdir(directorio);
	}

	closedir(directorio);
}

void completar_permisos(char permisos[9], mode_t mode) {
	if ((mode & S_IRUSR) == S_IRUSR)
		permisos[0] = 'r';
	if ((mode & S_IWUSR) == S_IWUSR)
		permisos[1] = 'w';
	if ((mode & S_IXUSR) == S_IXUSR)
		permisos[2] = 'x';
	if ((mode & S_IRGRP) == S_IRGRP)
		permisos[3] = 'r';
	if ((mode & S_IWGRP) == S_IWGRP)
		permisos[4] = 'w';
	if ((mode & S_IXGRP) == S_IXGRP)
		permisos[5] = 'x';
	if ((mode & S_IROTH) == S_IROTH)
		permisos[6] = 'r';
	if ((mode & S_IWOTH) == S_IWOTH)
		permisos[7] = 'w';
	if ((mode & S_IXOTH) == S_IXOTH)
		permisos[8] = 'x';
}

void mostrar_informacion(char* nombre) {

	struct stat sb;

    if (lstat(nombre, &sb) == -1) {
		perror("Error en lstat");
		exit(-1);
	}

	char permisos[11] = "----------";

	if (S_ISDIR(sb.st_mode))
		permisos[0] = 'd';
	else if (S_ISLNK(sb.st_mode)) {
		permisos[0] = 'l';

		char* linkname = malloc(sb.st_size + 1);
		if (linkname == NULL) {
			perror("Error en malloc");
			exit(-1);
		}

		ssize_t r = readlink(nombre, linkname, sb.st_size + 1);
		if (r == -1) {
		   perror("Error en readlink");
		   exit(-1);
		}

		if (r > sb.st_size) {
		   perror("Error en enlace simbolico");
		   exit(-1);
		}

		linkname[r] = '\0';
		strcat(nombre, " -> ");
		strcat(nombre, linkname);

		free(linkname);
	}

	completar_permisos(permisos+1, sb.st_mode);
	printf("%s  %ld  %s \n", permisos, (long) sb.st_uid, nombre);
}
