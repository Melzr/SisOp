#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

#ifndef PATH_MAX
#define PATH_MAX 1000
#endif

#define CASE_FLAG "-i"

/*
 * Concatena el path actual con el file_name y devuelve el resultado
 */
char *file_path(char *path, char *file_name);


/*
 * Busca recursivamente a partir del directorio actual archivos y subdirectorios
 * que contengan a cadena
 */
void buscar(int fds, char *cadena, char *(*comparador)(const char *, const char *) );

void buscar_aux(int fds,
                char path[PATH_MAX],
                char *cadena,
                char *(*comparador)(const char *, const char *) );


int
main(int argc, char *argv[])
{
	if (argc < 2 || argc > 3) {
		printf("Error: se debe ingresar una cadena de caracteres y/o "
		       "el flag -i\n");
		exit(-1);
	}

	DIR *directorio = opendir(".");
	if (directorio == NULL) {
		perror("Error en opendir");
		exit(-1);
	}

	if ((strcmp(argv[1], CASE_FLAG) == 0) && (argc == 3)) {
		buscar(dirfd(directorio), argv[2], strcasestr);
	} else {
		buscar(dirfd(directorio), argv[1], strstr);
	}

	closedir(directorio);

	return 0;
}

char *
file_path(char *path, char *file_name)
{
	if (strlen(path) > 0)
		strcat(path, "/");
	return strcat(path, file_name);
}

void
buscar(int fds, char *cadena, char *(*comparador)(const char *, const char *) )
{
	char path[PATH_MAX] = "\0";
	buscar_aux(fds, path, cadena, comparador);
}

void
buscar_aux(int fds,
           char path[PATH_MAX],
           char *cadena,
           char *(*comparador)(const char *, const char *) )
{
	DIR *directorio = fdopendir(fds);
	if (directorio == NULL) {
		perror("Error en fdopendir");
		return;
	}

	struct dirent *entry = readdir(directorio);

	while (entry != NULL) {
		char path_aux[PATH_MAX];
		strcpy(path_aux, path);

		if (entry->d_type == DT_DIR) {
			if ((strcmp(entry->d_name, ".") != 0) &&
			    (strcmp(entry->d_name, "..") != 0)) {
				file_path(path_aux, entry->d_name);
				if (comparador(entry->d_name, cadena) != NULL)
					printf("%s\n", path_aux);

				int fds_dir = openat(dirfd(directorio),
				                     entry->d_name,
				                     O_DIRECTORY);
				buscar_aux(fds_dir, path_aux, cadena, comparador);
			}

		} else {
			if (comparador(entry->d_name, cadena) != NULL)
				printf("%s\n", file_path(path_aux, entry->d_name));
		}

		entry = readdir(directorio);
	}

	closedir(directorio);
}
