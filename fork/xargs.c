#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#ifndef NARGS
#define NARGS 4
#endif

/*
 * Llena el vector argumentos con el comando recibido en la primera posicion,
 * NULL en la ultima y el resto con las lineas leidas de archivo sin el '\n',
 * los cuales estaran almacenados en el heap.
 * Devuelve la cantidad de caracteres leidos por ultima vez o -1 en caso de
 * error incluyendo final de archivo.
 */
int obtener_argumentos(char *argumentos[NARGS + 2], char *comando, FILE **archivo);

/*
 * Creara un proceso por cada NARGS lineas de archivo, cada procedo ejecutara
 * el comando recibido con los respectivos argumentos.
 */
void ejecutar_comando(char *comando, FILE **archivo);


int
main(int argc, char *argv[])
{
	if (argc < 2) {
		printf("Error: se necesita un argumento\n");
		exit(-1);
	}

	// FILE* archivo = fopen("prueba.txt", "r");
	// ejecutar_comando(argv[1], archivo);
	// fclose(archivo);

	ejecutar_comando(argv[1], &stdin);

	return 0;
}


int
obtener_argumentos(char *argumentos[NARGS + 2], char *comando, FILE **archivo)
{
	argumentos[0] = comando;

	char *linea = NULL;
	size_t n = 0;
	int i = 1;
	int lectura = getline(&linea, &n, *archivo);

	while ((i <= NARGS) && (lectura > 0)) {
		if (linea[strlen(linea) - 1] == '\n') {
			linea[strlen(linea) - 1] = '\0';
		}

		argumentos[i] = malloc(sizeof(char) * (strlen(linea) + 1));
		if (argumentos == NULL) {
			free(linea);
			perror("Error en malloc");
			exit(-1);
		}
		strcpy(argumentos[i], linea);

		if (i < NARGS)
			lectura = getline(&linea, &n, *archivo);
		i++;
	}

	argumentos[i] = NULL;
	free(linea);

	return lectura;
}


void
ejecutar_comando(char *comando, FILE **archivo)
{
	char *argumentos[NARGS + 2];
	int lectura = obtener_argumentos(argumentos, comando, archivo);
	int f = fork();

	if (f < 0) {
		perror("Error en fork");
		exit(-1);
	}

	if (f == 0) {
		execvp(argumentos[0], argumentos);
		perror("Error en execvp");
		_exit(-1);
	} else {
		int estado = -1;
		wait(&estado);
		if (!WIFEXITED(estado)) {
			perror("Error en uno de los procesos");
			_exit(-1);
		}
	}

	int i = 1;
	while ((i < (NARGS + 2)) && (argumentos[i] != NULL)) {
		free(argumentos[i]);
		i++;
	}

	if (lectura > 0)
		ejecutar_comando(comando, archivo);
}
