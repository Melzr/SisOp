# Lab shell

El propósito de este lab es el de desarrollar la funcionalidad mínima que caracteriza a un intérprete de comandos shell similar a lo que realizan bash, zsh, fish.

La implementación debe realizarse en C11 y POSIX.1-2008. (Estas siglas hacen referencia a la versión del lenguaje C utilizada y del estándar de syscalls Unix empleado. Las versiones modernas de GCC y Linux cumplen con ambos requerimientos.)

**Esqueleto**

Se provee un esqueleto de shell. Éste tiene gran parte del parseo hecho, y está estructurado indicando con comentarios los lugares en donde deben introducir el código crítico de cada punto.

**Depurado con printf**

Es importante mencionar que es requisito usar las funciones printf_debug y fprintf_debug si se desea mostrar información por pantalla; o bien encapsular todo lo que se imprima por stdout o stderr utilizando la macro SHELL_NO_INTERACTIVE (como ejemplo, ver las funciones definidas en utils.c).

Cualquier mensaje que se imprima por pantalla al momento de hacer la entrega tiene que hacerse con las funciones printf_debug (en lugar de printf) o bien encapsulando el código con la directiva del preprocesador #ifndef SHELL_NO_INTERACTIVE.

<br/>

## Parte 1: Invocación de comandos

### Búsqueda en $PATH

Los comandos que usualmente se utilizan, como los realizados en el lab anterior, están guardados (sus binarios), en el directorio /bin. Por este motivo existe una variable de entorno llamada $PATH, en la cual se declaran las rutas más usualmente accedidas por el sistema operativo (ejecutar: `echo $PATH` para ver la totalidad de las rutas almacenadas). Se pide agregar la funcionalidad de poder invocar comandos, cuyos binarios se encuentren en las rutas especificadas en la variable $PATH.

    $ uptime
    05:45:25 up 5 days, 12:02,  5 users,  load average: ...

- Implementar: Ejecución de programas.
- Responder: ¿cuáles son las diferencias entre la syscall execve(2) y la familia de wrappers proporcionados por la librería estándar de C (libc) exec(3)?
- Responder: ¿Puede la llamada a exec(3) fallar? ¿Cómo se comporta la implementación de la shell en ese caso?

**Argumentos del programa**

En esta parte del lab, vamos a incorporar a la invocación de comandos, la funcionalidad de poder pasarle argumentos al momento de querer ejecutarlos. Los argumentos pasados al programa de esta forma, se guardan en la famosa variable *char* argv[]*, junto con cuántos fueron en *int argc*, declaradas en el main de cualquier programa en C.

    $ df -H /tmp
    Filesystem      Size  Used Avail Use% Mounted on
    tmpfs           8.3G  2.0M  8.3G   1% /tmp

- Implementar: Ejecución de programas con argumentos (argv[]).

Función sugerida: execvp(3)

Archivo: exec_cmd() en exec.c

### Procesos en segundo plano

Los procesos en segundo plano o procesos en el fondo, o background, son muy útiles a la hora de ejecutar comandos que no queremos esperar a que terminen para que la shell nos devuelva el prompt nuevamente. Por ejemplo si queremos ver algún documento .pdf o una imagen y queremos seguir trabajando en la terminal sin tener que abrir una nueva.

    $ evince file.pdf &
    [PID=2489]

    $ ls /home
    patricio

Sólo se pide la implementación de un proceso en segundo plano. No es necesario que se notifique de la terminación del mismo por medio de mensajes en la shell.

Sin embargo, la shell deberá *esperar oportunamente* a los procesos en segundo plano. Esto puede hacerse sincrónicamente antes de mostrar cada prompt, pero el objetivo es que en una ejecución normal no se dejen procesos huérfanos.

- Implementar: Procesos en segundo plano. Sin notificación de procesos terminados, pero esperando oportunísticamente con cada prompt a los procesos en segundo plano.
- Responder: Detallar cuál es el mecanismo utilizado para implementar procesos en segundo plano.

Ayuda: Leer el funcionamiento del flag WNOHANG de la syscall wait(2)

### Resumen

Al finalizar la parte 1 la shell debe poder:

- Invocar programas y permitir pasarles argumentos.
- Esperar correctamente a la ejecución de los programas.
- Ejecutar procesos en segundo plano.
- Esperar oportunísticamente a los procesos en segundo plano antes de cada prompt.

<br/>

## Parte 2: Redirecciones

### Flujo estándar

La redirección del flujo estándar es una de las cualidades más interesantes y valiosas de una shell moderna. Permite, entre otras cosas, almacenar la salida de un programa en un archivo de texto para luego poder analizarla, como así también ejecutar un programa y enviarle un archivo a su entrada estándar. Existen, básicamente tres formas de redirección del flujo estándar:

- **Entrada y Salida estándares a archivos (`<in.txt >out.txt`)**

Son los operadores clásicos del manejo de la redirección del stdin y el stdout en archivos de entrada y salida respectivamente. Por ejemplo:

    $ ls /usr
    bin etc games include lib local sbin share src

    $ ls /usr >out1.txt
    $ cat out1.txt
    bin etc games include lib local sbin share src

    $ wc -w <out1.txt
    10

    $ ls -C /sys /noexiste >out2.txt
    ls: cannot access '/noexiste': No such file or directory

    $ cat out2.txt
    /sys:
    block  class  devices	 fs  kernel  module  power

    $ wc -w <out2.txt
    8

Se puede ver cómo queda implícito que cuando se utiliza el operador > se refiere al stdout y cuando se utiliza el < se refiere al stdin.

- **Error estándar a archivo (`2>err.txt`)**

Es una de las dos formas de redireccionar el flujo estándar de error análogo al caso anterior del flujo de salida estándar en un archivo de texto. Por ejemplo:

    $ ls -C /home /noexiste >out.txt 2>err.txt

    $ cat out.txt
    /home:
    patricio

    $ cat err.txt
    ls: cannot access '/noexiste': No such file or directory

Como se puede observar, ls no informa ningún error al finalizar, como sí lo hacía en el ejemplo anterior. Su salida estándar de error ha sido redireccionada al archivo err.txt

- **Combinar salida y errores (`2>&1`)**

Es la segunda forma de redireccionar el flujo estándar producido por errores en la ejecución de un programa. Su funcionamiento se puede observar a través del siguiente ejemplo:

    $ ls -C /home /noexiste >out.txt 2>&1

    $ cat out.txt
    ---????---

Existen más tipos de redirecciones que nuestra shell no soportará (e.g. `>>` o `&>`)

- Implementar: Al menos, soporte para cada una de las tres formas de redirección descritas arriba: >, <, 2> y 2>&1.
- Responder: Investigar el significado de 2>&1, explicar cómo funciona su forma general y mostrar qué sucede con la salida de cat out.txt en el ejemplo. Luego repetirlo invertiendo el orden de las redirecciones. ¿Cambió algo?

Ayuda: Pueden valerse de las páginas del manual de bash: man bash.

Syscalls sugeridas: dup2(2), open(2)

Archivo: open_redir_fd() en exec.c y usarla en exec_cmd()

### Tuberías simples (pipes)

Al igual que la redirección del flujo estándar hacia archivos, es igual o más importante, la redirección hacia otros programas. La forma de hacer esto en una shell es mediante el operador `|` (pipe o tubería). De esta forma se pueden concatenar dos o más programas para que la salida estándar de uno se redirija a la entrada estándar del siguiente. Por ejemplo:

    $ ls -l | grep Doc
    drwxr-xr-x  7 patricio patricio  4096 mar 26 01:20 Documentos

- Implementar: Soporte para pipes entre dos comandos.
    - La shell debe esperar a que ambos procesos terminen antes de devolver el prompt: `echo hi | sleep 5` y `sleep 5 | echo hi` ambos deben esperar 5 segundos.
    - Los procesos de cada lado del pipe no deben quedar con fds de más.
    - Los procesos deben ser lanzados en simultáneo.

Syscalls sugeridas: pipe(2), dup2(2)

Archivo: exec_cmd() en exec.c

### Tuberías múltiples

Extender el funcionamiento de la shell para que se puedan ejecutar n comandos concatenados.

    $ ls -l | grep Doc | wc
        1       9      64

- Implementar: Soporte para múltiples pipes anidados.
- Responder: Investigar qué ocurre con el exit code reportado por la shell si se ejecuta un pipe ¿Cambia en algo? ¿Qué ocurre si, en un pipe, alguno de los comandos falla? Mostrar evidencia (e.g. salidas de terminal) de este comportamiento usando bash. Comparar con la implementación del este lab.

Hint: Las modificaciones necesarias sólo atañen a la función parse_line() en parsing.c

### Resumen

Al finalizar la parte 2 la shell debe poder:

- Redireccionar la entrada y salida estándar de los programas vía <, > y 2>.
- Además se soporta específicamente la redirección de tipo 2>&1
- Concatenar la ejecución de dos o más programas mediante pipes

<br/>

## Parte 3: Variables de entorno

### Expansión de variables

Una característica de cualquier intérprete de comandos shell es la de expandir variables de entorno (ejecutar: `env` para ver una lista completa de las varibles de entorno definidas), como PATH, o HOME.

    $ echo $TERM
    xterm-16color

Las variables de entorno se indican con el caracter `$` antes del nombre, y la shell se encarga de reemplazar en la línea leída todos los tokens que comiencen por `$` por los valores correspondientes del entorno. Esto ocurre antes de que el proceso sea ejecutado.

- Implementar: Expansión de variables al ejecutar un comando. Se debe reemplazar las variables que no existan con una cadena vacía ("").
- 
Función sugerida: getenv(3)

Archivo: expand_environ_var() en parsing.c

### Variables de entorno temporarias

En esta parte se va a extender la funcionalidad de la shell para que soporte el poder incorporar nuevas variables de entorno a la ejecución de un programa. Cualquier programa que hagamos en C, por ejemplo, tiene acceso a todas las variables de entorno definidas mediante la variable externa environ (`extern char** environ`).
Se pide, entonces, la posibilidad de incorporar de forma dinámica nuevas variables, por ejemplo:

    $ /usr/bin/env
    --- todas las variables de entorno definidas hasta el momento ---

    $ USER=nadie ENTORNO=nada /usr/bin/env | grep =nad
    USER=nadie
    ENTORNO=nada

- Implementar: Variables de entorno temporales.
- Pregunta: ¿Por qué es necesario hacerlo luego de la llamada a fork(2)?
- Pregunta: En algunos de los wrappers de la familia de funciones de exec(3) (las que finalizan con la letra e), se les puede pasar un tercer argumento (o una lista de argumentos dependiendo del caso), con nuevas variables de entorno para la ejecución de ese proceso. Supongamos, entonces, que en vez de utilizar setenv(3) por cada una de las variables, se guardan en un array y se lo coloca en el tercer argumento de una de las funciones de exec(3).
    - ¿El comportamiento resultante es el mismo que en el primer caso? Explicar qué sucede y por qué.
    - Describir brevemente (sin implementar) una posible implementación para que el comportamiento sea el mismo.

Ayuda: luego de llamar a fork(2) realizar, por cada una de las variables de entorno a agregar, una llamada a setenv(3).

Función sugerida: setenv(3)

Archivo: implementar set_environ_vars() en exec.c y usarla en exec_cmd()

### Pseudo-variables

Existen las denominadas variables de entorno mágicas, o pseudo-variables. Estas variables son propias del shell (no están formalmente en environ) y cambian su valor dinámicamente a lo largo de su ejecución. Implementar `?` como única variable mágica (describir, también, su próposito).

    $ /bin/true
    $ echo $?
    0

    $ /bin/false
    $ echo $?
    1

- Implementar: Soporte para para la pseudo-variable $?. Esto implicará actualizar correctamente la variable global status cuando se ejecute un built-in (ya que los mismos no corren en procesos separados).
- Pregunta: Investigar al menos otras tres variables mágicas estándar, y describir su propósito. Incluir un ejemplo de su uso en bash (u otra terminal similar).

Archivo: expand_environ_var() en parsing.c, ver también la variable global status.

### Resumen

Al finalizar la parte 3 la shell debe poder:

- Expandir variables de entorno
- Incluyendo la pseudo-variable $?
- Ejecutar procesos con variables de entorno adicionales

<br/>

## Parte 4: Extras

### Comandos built-in

Los comandos built-in nos dan la opurtunidad de realizar acciones que no siempre podríamos hacer si ejecutáramos ese mismo comando en un proceso separado. Éstos son propios de cada shell aunque existe un estándar generalizado entre los diferentes intérpretes, como por ejemplo `cd` y `exit`.

Es evidente que si `cd` no se realizara en el mismo proceso donde la shell se está ejecutando, no tendría el efecto deseado, ya que el directorio actual se cambiaría en el hijo, y no en el padre que es lo que realmente queremos. Lo mismo se aplica a `exit` y a muchos comandos más.

- Implementar: Los built-ins:
    - cd - change directory (cambia el directorio actual)
    - exit - exits nicely (termina una terminal de forma linda)
    - pwd - print working directory (muestra el directorio actual de trabajo)
- Pregunta: ¿Entre *cd* y *pwd*, alguno de los dos se podría implementar sin necesidad de ser built-in? ¿Por qué? ¿Si la respuesta es sí, cuál es el motivo, entonces, de hacerlo como built-in? (para esta última pregunta pensar en los built-in como *true* y *false*)
  
Funciones sugeridas: chdir(3), exit(3), getcwd(3)

Archivo: cd(), exit_shell() y pwd() en builtin.c