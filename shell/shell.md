# Lab: shell

### Búsqueda en $PATH

#### ¿Cuáles son las diferencias entre la syscall execve(2) y la familia de wrappers proporcionados por la librería estándar de C (libc) exec(3)?

La syscall execve(2) ejecuta un archivo recibido, sobreescribiendo el espacio de memoria del proceso que la llamó (stack, heap, data, código), sin cambiar su PID. Esto lo provee es sistema operativo.

La familia de exec(3) hacen un llamado a esa syscall proporcionando al programador formas más convenientes de usarla, como por ejemplo, no tener que especificar las variables de entorno, poder pasarles los argumentos individualmente o que realicen la búsqueda del archivo en PATH. 

####  ¿Puede la llamada a exec(3) fallar? ¿Cómo se comporta la implementación de la shell en ese caso?

exec(3) puede fallar, en cuyo caso como no se reemplazó el espacio de memoria del proceso (incluído el código), se ejecuta la siguiente línea del código de la shell, donde se avisa al usuario del error y termina el proceso con con un código de error. Luego de esto la shell seguirá en ejecución esperando un nuevo comando.

---

### Procesos en segundo plano

#### Detallar cuál es el mecanismo utilizado para implementar procesos en segundo plano.

Para implementar los procesos en segundo plano obtengo el proceso almacenado en `backcmd->c` y llamo recursivamente a `exec_cmd()`, por lo que se comportará de la misma manera que un proceso normal, con la única diferencia que en `run_cmd()` no se espera al proceso de manera bloqueante. Para no dejar a estos procesos en estado zombie, cada vez que se ingresa un comando la shell esperará de manera no bloqueante a cualquier proceso que se estuviera ejecutando en segundo plano.

---

### Flujo estándar

#### Investigar el significado de 2>&1, explicar cómo funciona su forma general y mostrar qué sucede con la salida de cat out.txt en el ejemplo. Luego repetirlo invertiendo el orden de las redirecciones. ¿Cambió algo?

`>` indica redireccion, el `2` es el file descriptor de stderr, e `&1` indica usar el mismo file descriptor de stdout, por lo que `2>&1` significa "redirigir la salida de error a la salida estandar".

```console
melina@PClinux ~ $ ls -C /home /noexiste >out.txt 2>&1
melina@PClinux ~ $ cat out.txt
ls: cannot access '/noexiste': No such file or directory
/home:
melina
```
Tanto la salida estandar como la salida de error se redirigieron a cat.txt.


```console
melina@PClinux ~ $ ls -C /home /noexiste 2>&1 >out.txt
ls: cannot access '/noexiste': No such file or directory
melina@PClinux ~ $ cat out.txt
/home:
melina
```
Al invertir el orden de las redirecciones, lo primero que se hace es redirigir la salida de error a la salida estadar, que por el momento es la teminal, y luego se redirige la salida estandar al archivo out.txt y se ejecuta el programa. No se vuelve a redirigir stderr.

---

### Tuberías múltiples

#### Investigar qué ocurre con el exit code reportado por la shell si se ejecuta un pipe ¿Cambia en algo? ¿Qué ocurre si, en un pipe, alguno de los comandos falla? Mostrar evidencia (e.g. salidas de terminal) de este comportamiento usando bash. Comparar con la implementación del este lab.

El exit code al ejecutar un pipe es el del ultimo comando, a menos que se use la opcion pipefail, en cuyo caso el exit code es el del ultimo comando que fallo o 0 si no fallo ninguno. 

<br/>
        
**Ejemplo 1 bash**
```console
melina@PClinux ~ $ ls -l | grep no_existe
melina@PClinux ~ $ echo $?
1
```
El exit code reportado por la shell es el del ultimo comando, ya que `ls -l` devuelve 0 y `grep` de algo que no existe devuelve 1, que es efectivamente el exit code del comando entero.  

<br/>

**Ejemplo 2 bash**
```console
melina@PClinux ~ $ ls -l | grep no_existe | echo hola
hola
melina@PClinux ~ $ echo $?
0
```
Al ejemplo anterior, se le agrega el comando `echo hola`, que imprime hola y termina con 0, que es el exit code del comando entero.

<br/>

**Ejemplo 3 bash**
```console
melina@PClinux ~ $ set -eo pipefail
melina@PCliunx ~ $ ls -l | grep no_existe | echo hola
hola
melina@PClinux ~ $ echo $?
1
```
Mismo comando que en el ejemplo anterior pero con la opcion pipefail activada, el exit code es 1 a pesar de que el ultimo comando termina con 0, ya que 1 es el exit code del ultimo comando que fallo, `grep no_existe`.

Este ultimo ejemplo no sera replicado en la shell del lab ya que no fue implementada esta opcion.


<br/>

**Ejemplo lab shell**
```console
 (/home/melina) 
$ ls -l | grep no_existe
 (/home/melina) 
$ echo $?
1
	Program: [echo $?] exited, status: 0 
 (/home/melina) 
$ ls -l | grep no_existe | echo hola
hola
 (/home/melina) 
$ echo $?
0
	Program: [echo $?] exited, status: 0 
```
En la implementacion de shell el exit code en cada caso coincide con el de los ejemplos 1 y 2, su comportamiento es el esperado.


---

### Variables de entorno temporarias

#### ¿Por qué es necesario hacerlo luego de la llamada a fork(2)?

Porque al ser variables temporales, solo deben existir en la ejecucion del programa y no en la shell misma. Por lo tanto, se deben setear luego del fork y antes de execv, para que solo esten definidas en el proceso que se va a transformar en el programa a ejecutar.


#### En algunos de los wrappers de la familia de funciones de exec(3) (las que finalizan con la letra e), se les puede pasar un tercer argumento (o una lista de argumentos dependiendo del caso), con nuevas variables de entorno para la ejecución de ese proceso. Supongamos, entonces, que en vez de utilizar setenv(3) por cada una de las variables, se guardan en un array y se lo coloca en el tercer argumento de una de las funciones de exec(3).

#### ¿El comportamiento resultante es el mismo que en el primer caso? Explicar qué sucede y por qué.
    
El comportamiento no es el mismo, lo que sucede es que el proceso que ejecuta el programa solo tiene como variables de entorno a las recibidas como argumento como variables de entorno temporarias (las almacenadas en `cmd->eargv`).

#### Describir brevemente (sin implementar) una posible implementación para que el comportamiento sea el mismo. 
    
Una posible implementacion puede ser usar la variable global `extern char **environ`, que es un array con las variables de entorno. Agregandole las variables de entorno temporales recibidas como argumento, al pasarselo como parametro a una de las funciones de la familia de funciones de exec(3) (las que finalizan con la letra e) obtendria el comportamiento esperado.


---

### Pseudo-variables

#### Investigar al menos otras tres variables mágicas estándar, y describir su propósito. Incluir un ejemplo de su uso en bash (u otra terminal similar).

##### 1. `$!`: expande al PID del ultimo proceso que se ejecuto en segundo plano

Ejemplo:
 ```console
melina@PClinux ~ $ echo hola &
[1] 17936
hola
melina@PClinux ~ $ echo hola
hola
[1]+  Done                    echo hola
melina@PClinux ~ $ echo $!
17936
```

##### 2. `$_`: expande al ultimo argumento recibido por el ultimo comando ejecutado

Ejemplo:
```console
melina@PClinux ~ $ echo hola como estas
hola como estas
melina@PClinux ~ $ echo $_
estas
```

##### 3. `$$`: espande al PID de la shell

Ejemplo:
```console
melina@PClinux ~ $ ps
  PID TTY          TIME CMD
17922 pts/0    00:00:00 bash
18185 pts/0    00:00:00 ps
melina@PClinux ~ $ echo $$
17922
```

---

### Comandos built-in

#### ¿Entre cd y pwd, alguno de los dos se podría implementar sin necesidad de ser built-in? ¿Por qué? ¿Si la respuesta es sí, cuál es el motivo, entonces, de hacerlo como built-in? (para esta última pregunta pensar en los built-in como true y false)

pwd se puede implementar sin ser un built-in. No es el caso de cd ya que si es implementado como un programa se estaría cambiando el directorio del proceso hijo y no el de la shell. 

La ventaja de implementar pwd como built-in es que será mucho más rápido al ahorrar el `fork()`, `exec()` y `wait()` que se requieren para ejecutar un programa.

---


