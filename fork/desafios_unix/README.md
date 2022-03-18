# Desafío: más utilidades Unix

* **ps**: implementación de `ps -eo pid,comm` . Se lista el pid y comando de cada proceso.
* **ls**: implementación de `ls -al` . Se listan los contenidos del directorio indicando para cada entrada tipo, permisos, id del usuario y nombre. En el caso de un enlace simbólico, se muestra el archivo/directorio al que apunta.
* **cp**: implementación de `cp src dst` donde `src` es un archivo regular que se copia a `dst` que no debe existir. 

## Compilar

```bash
$ make
```

## Linter

```bash
$ make format
```
