#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

/*
 * Devuelve true si el string recibido corresponde a un numero en base decimal
 */
bool es_numero(char name[256]);

/*
 * Recibibe un string correspondiente a un pid valido y lo imprime junto con su
 * comm
 */
void imprimir_proceso(char pid[]);


int
main()
{
	DIR *proc = opendir("/proc");
	if (proc == NULL) {
		perror("Error en opendir");
		exit(-1);
	}

	struct dirent *entry = readdir(proc);

	printf("    PID COMMAND\n");

	while (entry != NULL) {
		if (entry->d_type == DT_DIR && (es_numero(entry->d_name))) {
			imprimir_proceso(entry->d_name);
		}
		entry = readdir(proc);
	}

	closedir(proc);

	return 0;
}


bool
es_numero(char name[256])
{
	int i = 0;
	while ((i < 256) && (name[i] != '\0')) {
		if (isdigit(name[i]) == 0)
			return false;
		i++;
	}

	return true;
}


void
imprimir_proceso(char pid[])
{
	char ruta[300] = "/proc/";
	strcat(ruta, pid);
	strcat(ruta, "/comm");

	FILE *comando = fopen(ruta, "r");
	if (comando != NULL) {
		char *linea = NULL;
		size_t n = 0;
		int lectura = getline(&linea, &n, comando);

		if (lectura > 0)
			printf("%7s %s", pid, linea);

		fclose(comando);
		free(linea);
	} else {
		perror("Error: no se pudo abrir comms");
	}
}
