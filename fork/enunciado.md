# Lab fork

El objetivo de este lab es familiarizarse con las llamadas al sistema fork(2) (que crea una copia del proceso actual) y pipe(2) (que proporciona un mecanismo de comunicación unidireccional entre dos procesos).

## Tarea: pingpong

Se pide escribir un programa en C que use fork(2) y pipe(2) para enviar y recibir (ping-pong) un determinado valor entero, entre dos procesos. El valor se debe crear con random(3) una vez ambos procesos existan.

El programa debe imprimir por pantalla la secuencia de eventos de ambos procesos, en el formato exacto que se especifica a continuación:

    $ ./pingpong
    Hola, soy PID <x>:
    - primer pipe me devuelve: [3, 4]
    - segundo pipe me devuelve: [6, 7]

    Donde fork me devuelve <y>:
    - getpid me devuelve: <?>
    - getppid me devuelve: <?>
    - random me devuelve: <v>
    - envío valor <v> a través de fd=?

    Donde fork me devuelve 0:
    - getpid me devuelve: <?>
    - getppid me devuelve: <?>
    - recibo valor <v> vía fd=?
    - reenvío valor en fd=? y termino

    Hola, de nuevo PID <x>:
    - recibí valor <v> vía fd=?

Ayuda:

- Nótese que como las tuberías —pipes— son unidireccionales, se necesitarán dos para poder transmitir el valor en una dirección y en otra.

- Para obtener números aleatorios que varíen en cada ejecución del programa, se debe inicializar el PRNG (generador de números pseudo-aleatorios) mediante la función srandom(3) (típicamente con el valor de time(2)).

- Si fork(2) fallase, simplemente se imprime un mensaje por salida de error estándar (stderr), y el programa termina.

- Tener en cuenta el tipo de los valores de retorno para cada una de las syscalls/funciones de libc a utilizar (por ejemplo: random(3))

Llamadas al sistema: fork(2), pipe(2).

<br/>

## Tarea: primes

La criba de Eratóstenes (sieve of Eratosthenes en inglés) es un algoritmo milenario para calcular todos los primos menores a un determinado número natural, n.

Si bien la visualización del algoritmo suele hacerse “tachando” en una grilla, el concepto de criba, o sieve (literalmente: cedazo, tamiz, colador) debe hacernos pensar más en un filtro. En particular, puede pensarse en n filtros apilados, donde el primero filtra los enteros múltiplos de 2, el segundo los múltiplos de 3, el tercero los múltiplos de 5, y así sucesivamente.

Si modelásemos cada filtro como un proceso, y la transición de un filtro al siguiente mediante tuberías (pipes), se puede implementar el algoritmo con el siguiente pseudo-código (ver [fuente original](https://swtch.com/~rsc/thread/), y en particular la imagen que allí se muestra):

    p := <leer valor de pipe izquierdo>

    imprimir p // asumiendo que es primo

    mientras <pipe izquierdo no cerrado>:
        n = <leer siguiente valor de pipe izquierdo>
        si n % p != 0:
            escribir <n> en el pipe derecho

(El único proceso que es distinto, es el primero, que tiene que simplemente generar la secuencia de números naturales de 2 a n. No tiene lado izquierdo.)

La interfaz que se pide es:

    $ ./primes <n>

donde n será un número natural mayor o igual a 2. El código debe crear una estructura de procesos similar a la mostrada en la imagen, de tal manera que:

- El primer proceso cree un proceso derecho, con el que se comunica mediante un pipe.

- Ese primer proceso, escribe en el pipe la secuencia de números de 2 a n, para a continuación cerrar el pipe y esperar la finalización del proceso derecho.

- Todos los procesos sucesivos aplican el pseudo-código mostrado anteriormente, con la salvedad de que son responsables de crear a su “hermano” derecho, y la tubería (pipe) que los comunica.

- Se debería poder ejecutar correctamente el programa con un N mayor o igual a 10000.

Ejemplo de uso:

    $ ./primes 35
    primo 2
    primo 3
    primo 5
    primo 7
    primo 11
    primo 13
    primo 17
    primo 19
    primo 23
    primo 29
    primo 31

Ayuda:

- Conceptualmente esta tarea es la más difícil de las cuatro del lab, y no es prerrequisito para poder realizar las dos siguientes.

Llamadas al sistema: fork(2), pipe(2).

<br/>

## Tarea: find

Se pide escribir una versión muy simplificada de la utilidad find(1). Esta herramienta, tal y como se la encuentra en sistemas GNU/Linux, acepta una miríada de opciones (ver su [página de manual](https://dashdash.io/1/find)). No obstante, en este lab se implementará sólo una de ellas.

La sinopsis de nuestra implementación será:

    $ ./find [-i] <cadena>

Invocado como `./find xyz`, el programa buscará y mostrará por pantalla todos los archivos del directorio actual (y subdirectorios) cuyo nombre contenga (o sea igual a) `xyz`. Si se invoca como `./find -i xyz`, se realizará la misma búsqueda, pero sin distinguir entre mayúsculas y minúsculas.

Por ejemplo, si en el directorio actual se tiene:

    .
    ├── Makefile
    ├── find.c
    ├── xargs.c
    ├── antiguo
    │   ├── find.c
    │   ├── xargs.c
    │   ├── pingpong.c
    │   ├── basurarghh
    │   │   ├── find0.c
    │   │   ├── find1.c
    │   │   ├── pongg.c
    │   │   └── findddddddd.c
    │   ├── planes.txt
    │   └── pingpong2.c
    ├── antinoo.jpg
    └── GNUmakefile

el resultado de las distintas invocaciones debe ser como sigue (no importa el orden en que se impriman los archivos de un mismo directorio):

    $ ./find akefile
    Makefile
    GNUmakefile

    $ ./find Makefile
    Makefile

    $ ./find -i Makefile
    Makefile
    GNUmakefile

    $ ./find arg
    xargs.c
    antiguo/xargs.c
    antiguo/basurarghh

    $ ./find pong
    antiguo/pingpong.c
    antiguo/basurarghh/pongg.c
    antiguo/pingpong2.c

    $ ./find an
    antiguo
    antiguo/planes.txt
    antinoo.jpg

    $ ./find d.c
    find.c
    antiguo/find.c
    antiguo/basurarghh/findddddddd.c

Ayuda:

- Usar recursividad para descender a los distintos directorios.

- Nunca descender los directorios especiales . y .. (ambos son un “alias”; el primero al directorio actual, el segundo a su directorio inmediatamente superior).

- No es necesario preocuparse por ciclos en enlaces simbólicos.

- En el resultado de readdir(3), asumir que el campo d_type siempre está presente, y es válido.

- La implementación case-sensitive vs. case-insensitive (opción -i) se puede resolver limpiamente usando un puntero a función como abstracción. (Ver strstr(3).)

Requisitos:

- Llamar a la función opendir(3) una sola vez, al principio del programa (con argumento "."; no es necesario conseguir el nombre del directorio actual, si tenemos su alias).

- Para abrir sub-directorios, usar exclusivamente la función openat(2) (con el flag O_DIRECTORY como precaución). De esta manera, no es necesario realizar concatenación de cadenas para abrir subdirectorios.

    -Sí será necesario, no obstante, concatenar cadenas para mostrar por pantalla los resultados. No es necesario usar memoria dinámica; es suficiente un único buffer estático de longitud PATH_MAX.

    -Funciones que resultarán útiles como complemento a openat(): dirfd(3), fdopendir(3).

Llamadas al sistema: openat(2), readdir(3).

<br/>

## Tarea: xargs

En su versión más sencilla, la utilidad `xargs(1)` permite:

- Tomar un único argumento (`argv[1]`, un sólo comando).
- Leer nombres de archivos de la entrada estándar (stdin), de a uno por línea.
- Para cada nombre de archivo leído, invocar al comando especificado con el nombre de archivo como argumento.

Por ejemplo, si se invoca:

    $ echo /home | xargs ls

Esto sería equivalente a realizar `ls /home`. Pero si se invoca:

    $ printf "/home\n/var\n" | xargs ls

Esto sería equivalente a ejecutar `ls /home; ls/var`.

Variantes (soportadas por la utilidad):

- Aceptar más de un argumento. Por ejemplo:

    $ printf "/home\n/var\n" | xargs ls -l

En este caso, el comando ejecutado sobre el input es `ls -l`.

- Aceptar nombres de archivos separados por espacio, por ejemplo:

    $ echo /home /var | xargs ls

(Esto impediría, no obstante, que se puedan pasar a xargs nombres de archivos con espacios en sus nombres, como ser: `/home/user/Media Files`)

- Aceptar el “empaquetado” de varios nombres de archivos en una sola invocación del comando. Por ejemplo, en el segundo ejemplo de arriba, que se ejecute `ls /home /var` (una sola vez) en lugar de `ls /home; ls /var` (dos veces).

Se pide implementar la siguiente versión modificada de xargs.

Requisitos:

- Leer los nombres de archivo línea a línea, nunca separados por espacios (se recomienda usar la función getline(3)). También, es necesario eliminar el caracter `'\n'` para obtener el nombre del archivo.

- El “empaquetado” vendrá definido por un valor entero positivo disponible en la macro `NARGS` (la cual tiene efecto durante el proceso de compilación). El programa debe funcionar de manera tal que siempre se pasen `NARGS` argumentos al comando ejecutado (excepto en su última ejecución, que pueden ser menos). Si no se encuentra definida `NARGS`, *se debe definir a 4*. Se puede hacer algo como:

    #ifndef NARGS
    #define NARGS 4
    #endif

- Se debe esperar siempre a que termine la ejecución del comando actual.

- Mejora o funcionalidad opcional: si el primer argumento a xargs es -P, emplear hasta 4 ejecuciones del comando en paralelo (pero nunca más de 4 a la vez).
  
Llamadas al sistema: fork(2), execvp(3), wait(2).