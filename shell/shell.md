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

---

### Tuberías múltiples

#### Investigar qué ocurre con el exit code reportado por la shell si se ejecuta un pipe ¿Cambia en algo? ¿Qué ocurre si, en un pipe, alguno de los comandos falla? Mostrar evidencia (e.g. salidas de terminal) de este comportamiento usando bash. Comparar con la implementación del este lab.

---

### Variables de entorno temporarias

#### ¿Por qué es necesario hacerlo luego de la llamada a fork(2)?



#### En algunos de los wrappers de la familia de funciones de exec(3) (las que finalizan con la letra e), se les puede pasar un tercer argumento (o una lista de argumentos dependiendo del caso), con nuevas variables de entorno para la ejecución de ese proceso. Supongamos, entonces, que en vez de utilizar setenv(3) por cada una de las variables, se guardan en un array y se lo coloca en el tercer argumento de una de las funciones de exec(3).

  - ¿El comportamiento resultante es el mismo que en el primer caso? Explicar qué sucede y por qué.
  - Describir brevemente (sin implementar) una posible implementación para que el comportamiento sea el mismo. 



---

### Pseudo-variables

#### Investigar al menos otras tres variables mágicas estándar, y describir su propósito. Incluir un ejemplo de su uso en bash (u otra terminal similar).

---

### Comandos built-in

#### ¿Entre cd y pwd, alguno de los dos se podría implementar sin necesidad de ser built-in? ¿Por qué? ¿Si la respuesta es sí, cuál es el motivo, entonces, de hacerlo como built-in? (para esta última pregunta pensar en los built-in como true y false)

pwd se puede implementar sin ser un built-in. No es el caso de cd ya que si es implementado como un programa se estaría cambiando el directorio del proceso hijo y no el de la shell. 

La ventaja de implementar pwd como built-in es que será mucho más rápido al ahorrar el `fork()`, `exec()` y `wait()` que se requieren para ejecutar un programa.

---


