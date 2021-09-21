#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

#ifndef PATH_MAX
#define PATH_MAX 1000
#endif

#define CASE_FLAG "-i"

/*
 * Concatena el path actual con el file_name, sin modificarlos y devuelve el
 * resultado
 */
char *file_path(char *path, char *file_name);

/*
 * Imprime el path si nombre coincide con la cadena_buscada segun comparador
 */
void imprimir_si_buscado(char *nombre,
                         char *cadena_buscada,
                         char path_actual[PATH_MAX],
                         char *(*comparador)(const char *, const char *) );

/*
 * Busca recursivamente a partir del directorio actual archivos y subdirectorios
 * cuya comparacion con cadena no sea nula
 */
void buscar(int fds, char *cadena, char *(*comparador)(const char *, const char *) );

void buscar_aux(int fds,
                char path[PATH_MAX],
                char *cadena,
                char *(*comparador)(const char *, const char *) );


char *
file_path(char *path, char *file_name)
{
	char path_nuevo[PATH_MAX];
	strcpy(path_nuevo, path);

	if (strlen(path_nuevo) > 0)
		strcat(path_nuevo, "/");
	return strcat(path_nuevo, file_name);
}

void
imprimir_si_buscado(char *nombre,
                    char *cadena_buscada,
                    char path[PATH_MAX],
                    char *(*comparador)(const char *, const char *) )
{
	if (comparador(nombre, cadena_buscada) != NULL)
		printf("%s\n", path);
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
		if (entry->d_type == DT_DIR) {
			if ((strcmp(entry->d_name, ".") != 0) &&
			    (strcmp(entry->d_name, "..") != 0)) {
				char *path_dir = file_path(path, entry->d_name);
				imprimir_si_buscado(entry->d_name,
				                    cadena,
				                    path_dir,
				                    comparador);
				int fds_dir = openat(dirfd(directorio),
				                     entry->d_name,
				                     O_DIRECTORY);
				buscar_aux(fds_dir, path_dir, cadena, comparador);
			}
		} else {  // chequear si es DT_REG o otro?
			imprimir_si_buscado(entry->d_name,
			                    cadena,
			                    file_path(path, entry->d_name),
			                    comparador);
		}

		entry = readdir(directorio);
	}

	closedir(directorio);
}

int
main(int argc, char *argv[])
{
	if (argc < 2 || argc > 3) {
		printf("Error: se debe ingresar una cadena de caracteres y/o "
		       "el flag -i\n");
		return 0;
	}

	DIR *directorio = opendir(".");
	if (directorio == NULL) {
		perror("Error en opendir");
		return 0;
	}

	if ((strcmp(argv[1], CASE_FLAG) == 0) && (argc == 3)) {
		buscar(dirfd(directorio), argv[2], strcasestr);
	} else {
		buscar(dirfd(directorio), argv[1], strstr);
	}

	closedir(directorio);

	return 0;
}
