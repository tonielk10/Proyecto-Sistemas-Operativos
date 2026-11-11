# Simulador de kernel y gestor de procesos

Simulador de un kernel con gestor de procesos y planificador de CPU, controlado desde una consola interactiva. Está escrito en C++17 con `pthreads`: un hilo atiende la consola (CLI) y otro hilo (el Motor) hace avanzar un reloj simulado tick a tick. Soporta las políticas Round Robin (`RR`) y Shortest Remaining Time First (`SRTF`), diagrama de Gantt y métricas de espera, retorno y respuesta.

<!-- COMPLETAR: curso, universidad, semestre y profesor (el repositorio no trae estos datos) -->

## Requisitos

- `g++` con soporte de C++17 (o `clang++`: `make CXX=clang++`)
- `make`
- Sistema POSIX: Linux, WSL o macOS
- `valgrind` (opcional, solo para generar la evidencia de memoria y concurrencia)

## Compilación y ejecución

```bash
make            # compila la variante release en build/release/simulador
make run        # compila (si hace falta) y ejecuta el simulador
make debug      # -g -O0           -> build/debug/simulador
make tsan       # ThreadSanitizer  -> build/tsan/simulador
make asan       # Address + UBSan  -> build/asan/simulador
make test       # compila y ejecuta todas las pruebas (release)
make test-tsan  # igual, con ThreadSanitizer
make clean      # borra build/
make help       # lista los targets
```

Cada variante usa su propio directorio dentro de `build/`. La compilación es incremental y funciona con `make -j`.

## Comandos

Los comandos distinguen mayúsculas de minúsculas. Una línea admite hasta 1024 caracteres y no puede contener caracteres de control.

| Sintaxis | Qué hace | Rangos y valores por defecto | Ejemplo |
|---|---|---|---|
| `create <nombre> <burst_time> [prioridad]` | Crea un proceso que llega en el reloj actual y lo deja listo | Nombre de 1 a 32 caracteres, sin espacios ni comas. Burst entero de 1 a 1 000 000. Prioridad 1-5 (1 = más alta), por defecto 3 | `create P1 5 2` |
| `load <archivo>` | Carga procesos desde un archivo de trazas (todo o nada) | Ver [formato de trazas](#formato-de-load) | `load workloads/workload_test.txt` |
| `ps` | Lista todos los procesos con su estado | | `ps` |
| `pcb <PID>` | Muestra el bloque de control de un proceso | PID entero positivo | `pcb 1` |
| `set_sched <algoritmo> [quantum]` | Elige la política | `RR` o `SRTF`, en mayúsculas. Quantum entero 1-1000, por defecto 2 (solo aplica a `RR`; con `SRTF` se ignora) | `set_sched RR 3` |
| `set_speed <ms_por_tick>` | Duración real de cada tick | Entero de 50 a 5000 ms, por defecto 1000 | `set_speed 200` |
| `start` | Inicia o reanuda la simulación | | `start` |
| `pause` | Pausa al terminar el tick actual | | `pause` |
| `gantt` | Muestra el diagrama de Gantt de lo ejecutado | | `gantt` |
| `stats` | Muestra espera, retorno y respuesta promedio y uso de CPU | | `stats` |
| `help` | Muestra la ayuda | | `help` |
| `exit` | Cierra el simulador | | `exit` |

La simulación arranca en pausa. Los errores se muestran con el prefijo `[ERROR DE SINTAXIS]`, `[ERROR DE TIPO]`, `[ERROR DE PARAMETRO]` o `[ERROR]`.

### Formato de `load`

Un proceso por línea, con cuatro campos separados por comas:

```
# NOMBRE, BURST_TIME, PRIORIDAD, ARRIVAL_TIME
Job_A, 4, 2, 0
Job_B, 2, 1, 1
Job_C, 1, 3, 3
```

- Se ignoran las líneas vacías y las que empiezan por `#`; los espacios y tabs alrededor de los campos se recortan.
- `BURST_TIME` 1-1 000 000, `PRIORIDAD` 1-5, `ARRIVAL_TIME` 0-1 000 000.
- `ARRIVAL_TIME` es **relativo al reloj actual**: la llegada real es `reloj + ARRIVAL_TIME`.
- La carga es atómica: si alguna línea es inválida se informan los errores (con su número de línea) y no se carga ningún proceso. Máximo 10 000 procesos y 10 MB por archivo.
- En [`workloads/`](workloads/) hay archivos de ejemplo, válidos e inválidos; ver [`workloads/LEEME.md`](workloads/LEEME.md).

## Escenarios del enunciado

Las salidas de abajo son las que imprime el simulador realmente, ejecutado con `set_speed 50` y pausas (`sleep`) entre comandos. Para reproducirlas a mano, escribe los comandos uno a uno en `build/release/simulador`; con la velocidad por defecto (1000 ms) tardan 10 s y 7 s.

### Escenario 1: Round Robin, quantum 2

Comandos: `set_sched RR 2`, `set_speed 50`, `create P1 5 2`, `create P2 3 1`, `create P3 2 3`, `ps`, `start`, y cuando termina (`[INFO] Simulación finalizada`), `ps`, `gantt`, `stats`, `exit`.

```
> [OK] Política: RR (quantum=2)
> [OK] Velocidad: 50 ms/tick
> [OK] Proceso 'P1' creado exitosamente con PID 1
> [OK] Proceso 'P2' creado exitosamente con PID 2
> [OK] Proceso 'P3' creado exitosamente con PID 3
> PID    NOMBRE         ESTADO       BURST RESTANTE    PRIORIDAD
------------------------------------------------------------
1      P1             READY        5s                2
2      P2             READY        3s                1
3      P3             READY        2s                3
> [OK] Simulación iniciada.
> [INFO] Simulación finalizada: no quedan procesos pendientes.
PID    NOMBRE         ESTADO       BURST RESTANTE    PRIORIDAD
------------------------------------------------------------
1      P1             TERMINATED   0s                2
2      P2             TERMINATED   0s                1
3      P3             TERMINATED   0s                3
> DIAGRAMA DE GANTT (0 - 10 ticks):
[0-2s: P1] -> [2-4s: P2] -> [4-6s: P3] -> [6-8s: P1] -> [8-9s: P2] -> [9-10s: P1]
> ESTADÍSTICAS DEL SISTEMA:
- Procesos Finalizados        : 3 / 3
- Tiempo Promedio de Espera   : 5.00 ticks
- Tiempo Promedio de Retorno  : 8.33 ticks
- Tiempo Promedio de Respuesta: 2.00 ticks
- Uso de CPU                  : 100.0%
```

El Gantt coincide con el esperado.

### Escenario 1, variante: pausa y cambio a SRTF

Igual hasta `start`; después `pause`, `ps`, `pcb 1`, `set_sched SRTF`, `start`, y al terminar `ps`, `gantt`, `stats`, `exit`. El punto exacto donde cae `pause` depende del tiempo real entre comandos; en esta ejecución la pausa cayó tras 7 ticks, con P3 terminado:

```
> [OK] Simulación iniciada.
> [OK] Simulación en pausa (termina el tick actual).
> PID    NOMBRE         ESTADO       BURST RESTANTE    PRIORIDAD
------------------------------------------------------------
1      P1             RUNNING      2s                2
2      P2             READY        1s                1
3      P3             TERMINATED   0s                3
> ==================================================
        CONTROL BLOCK DE PROCESO (PCB) - PID: 1
==================================================
Nombre del Proceso : P1
Estado Actual      : RUNNING
Prioridad          : 2
Tiempo de Llegada  : 0 ticks
Ráfaga Inicial     : 5 ticks
Ráfaga Restante    : 2 ticks
Tiempo de Espera   : 4 ticks
Tiempo de Retorno  : N/A (En ejecución)
Tiempo de Respuesta: 0 ticks
==================================================
> [OK] Política: SRTF
> [OK] Simulación iniciada.
> [INFO] Simulación finalizada: no quedan procesos pendientes.
PID    NOMBRE         ESTADO       BURST RESTANTE    PRIORIDAD
------------------------------------------------------------
1      P1             TERMINATED   0s                2
2      P2             TERMINATED   0s                1
3      P3             TERMINATED   0s                3
> DIAGRAMA DE GANTT (0 - 10 ticks):
[0-2s: P1] -> [2-4s: P2] -> [4-6s: P3] -> [6-7s: P1] -> [7-8s: P2] -> [8-10s: P1]
> ESTADÍSTICAS DEL SISTEMA:
- Procesos Finalizados        : 3 / 3
- Tiempo Promedio de Espera   : 4.67 ticks
- Tiempo Promedio de Retorno  : 8.00 ticks
- Tiempo Promedio de Respuesta: 2.00 ticks
- Uso de CPU                  : 100.0%
```

Hasta el tick 7 el Gantt es el de Round Robin; luego SRTF elige a P2 (le queda 1 tick) antes que a P1 (le quedan 2).

### Escenario 2: SRTF con llegadas

Comandos: `set_sched SRTF`, `set_speed 50`, `load workloads/workload_test.txt`, `ps`, `start`, y al terminar `ps`, `gantt`, `stats`, `exit`.

```
> [OK] Política: SRTF
> [OK] Velocidad: 50 ms/tick
> [OK] Se cargaron 3 proceso(s) desde 'workloads/workload_test.txt' (PID 1 a 3).
[INFO] Usa 'start' para iniciar la simulación.
> PID    NOMBRE         ESTADO       BURST RESTANTE    PRIORIDAD
------------------------------------------------------------
1      Job_A          NEW          4s                2
2      Job_B          NEW          2s                1
3      Job_C          NEW          1s                3
> [OK] Simulación iniciada.
> [INFO] Simulación finalizada: no quedan procesos pendientes.
PID    NOMBRE         ESTADO       BURST RESTANTE    PRIORIDAD
------------------------------------------------------------
1      Job_A          TERMINATED   0s                2
2      Job_B          TERMINATED   0s                1
3      Job_C          TERMINATED   0s                3
> DIAGRAMA DE GANTT (0 - 7 ticks):
[0-1s: Job_A] -> [1-3s: Job_B] -> [3-4s: Job_C] -> [4-7s: Job_A]
> ESTADÍSTICAS DEL SISTEMA:
- Procesos Finalizados        : 3 / 3
- Tiempo Promedio de Espera   : 1.00 ticks
- Tiempo Promedio de Retorno  : 3.33 ticks
- Tiempo Promedio de Respuesta: 0.00 ticks
- Uso de CPU                  : 100.0%
```

El Gantt coincide con el esperado. En ambos escenarios las salidas reales coinciden con las esperadas del enunciado.

## Estructura del repositorio

```
.
├── Makefile
├── include/          # headers (un módulo por header)
├── src/              # implementaciones; main.cpp es el único main del simulador
├── tests/            # test_*.cpp (cada uno con su main), entradas/ y esperado/ de la prueba de consola
├── workloads/        # trazas de ejemplo para `load`
└── scripts/          # generar_evidencia.sh
```

El bloque de control de proceso está en [`include/PCB.h`](include/PCB.h). La evidencia generada irá en `docs/evidencia/` (ver más abajo).

| Módulo | Responsabilidad |
|---|---|
| `PCB` | Datos de un proceso y validación de transiciones de estado (NEW, READY, RUNNING, TERMINATED) |
| `Cli` | Bucle de lectura, validación de la línea y despacho a la tabla de comandos |
| `Comandos` (`ComandoCreate`, `ComandoLoad`, `ComandoPs`, `ComandoExit`, `ComandosControl`, `ComandosReportes`) | Un comando de la consola cada uno; `ComandosControl` agrupa start, pause, help, set_sched y set_speed; `ComandosReportes`, pcb, gantt y stats |
| `ContextoCli` | Referencias a las estructuras compartidas que reciben los comandos |
| `Mensajes` | Textos de error, ayuda y sintaxis (`USO_*`) |
| `Tokenizador`, `ParserEntero`, `ValidadorNombre` | Separar la línea, convertir enteros con rango y validar nombres |
| `Traza` | Parser puro de archivos de `load` (sin archivos ni hilos) |
| `GestorProcesos` | Crea procesos, asigna PIDs y centraliza los cambios de estado |
| `TablaProcesos` | Único lugar donde viven los PCB |
| `ColaListos` | Cola FIFO de PIDs en estado listo |
| `ListaLlegadas` | Procesos que aún no llegan, ordenados por llegada y PID |
| `EstadoSimulador` | Reloj, política, quantum, velocidad, pausa y apagado |
| `Motor` | Hilo que ejecuta los ticks y aplica la política |
| `PoliticaPlanificacion` | `RoundRobin` y `Srtf` como funciones puras de decisión |
| `RegistroGantt` | Segmentos del diagrama de Gantt |
| `Metricas` | Cálculo de espera, retorno, respuesta y uso de CPU |
| `Cerrojo`, `Salida` | Guard RAII de `pthread_mutex_t` y mutex de la salida a `std::cout` |

## Arquitectura de hilos y sincronización

Hay dos hilos: el **hilo CLI** (el principal) lee y ejecuta comandos, y el **hilo Motor** simula un tick por iteración. Se comunican únicamente a través de estructuras compartidas, cada una con su mutex (`pthread_mutex_t`, usado con el guard RAII `Cerrojo`).

- **Pausa sin busy-waiting:** `EstadoSimulador` tiene un mutex y una variable de condición; el Motor espera en `pthread_cond_wait` mientras la simulación está en pausa, dentro de un `while` para tolerar despertares espurios.
- **Nunca se duerme con un lock tomado:** el Motor duerme el tiempo de cada tick (en tramos de 20 ms, para poder apagarse rápido) sin ningún lock.
- **Sin locks anidados:** cada método de las estructuras toma un solo mutex y lo suelta al salir; el Motor coordina varias estructuras en pasos separados. Por eso no hay deadlock por orden de adquisición. La única excepción controlada es `mutex_salida`: el CLI lo mantiene mientras ejecuta un comando (puede tomar otros mutex dentro), y el Motor lo toma solo para imprimir y sin tener otro mutex.
- **Apagado:** `exit` termina el bucle del CLI; `main` llama a `Motor::detener_y_unir`, que pide el apagado, despierta al Motor y hace `pthread_join` antes de destruir los objetos.

| Dato compartido | Mutex que lo protege |
|---|---|
| Reloj, política, quantum, velocidad, pausa y apagado | mutex de `EstadoSimulador` (con su variable de condición) |
| PCB de todos los procesos | mutex de `TablaProcesos` |
| Cola de listos | mutex de `ColaListos` |
| Procesos pendientes de llegar | mutex de `ListaLlegadas` |
| Segmentos del Gantt | mutex de `RegistroGantt` |
| Contador de PID | mutex de `GestorProcesos` |
| Salida a `std::cout` desde ambos hilos | `mutex_salida` (`Salida.h`) |
| `pid_actual` y `quantum_usado` | ninguno: son locales del hilo Motor |

## Decisiones de diseño

- **Orden de un tick** (`Motor::bucle`): (1) pasan a la cola de listos los procesos con llegada ≤ reloj, antes de reencolar al desalojado; (2) se toma una foto del proceso actual y de la cola; (3) la política decide; (4) si cambia el proceso, el anterior vuelve a READY y entra al final de la cola; (5) el elegido consume un tick; (6) los procesos que esperan en la cola suman 1 de espera; (7) se registra el tick en el Gantt; (8) avanza el reloj; (9) se duerme `ms_por_tick`.
- **Métricas:** *WT* (espera) = ticks que el proceso pasó en la cola de listos. *TAT* (retorno) = tick de finalización − tiempo de llegada. *RT* (respuesta) = tick de la primera ejecución − tiempo de llegada. Los promedios se calculan solo sobre procesos terminados; el uso de CPU es ticks no ociosos / ticks totales del Gantt.
- **Desempates de SRTF:** menor tiempo restante; si empatan, el proceso actual; luego mayor prioridad (número menor); luego menor tiempo de llegada; por último menor PID. SRTF puede desalojar en cualquier tick.
- **Round Robin:** el proceso actual sigue mientras `quantum_usado < quantum`. Al agotarse el quantum, se elige el primero de la cola y el desalojado entra al final, detrás de los que llegaron en ese mismo tick. Si no hay nadie más en la cola, el actual continúa. Al cambiar de política se reinicia el quantum usado.
- **`ARRIVAL_TIME` en `load`:** es relativo al reloj en el momento de cargar (`reloj + ARRIVAL_TIME`), por lo que se puede cargar un archivo con la simulación ya en marcha.
- **IDLE:** si no hay proceso ejecutándose ni listo pero quedan llegadas pendientes, el tick se registra como `IDLE` en el Gantt (cuenta como CPU ociosa).
- **Pausa automática:** cuando no queda nada por ejecutar ni por llegar, el Motor pone la simulación en pausa e imprime `[INFO] Simulación finalizada: no quedan procesos pendientes.`; un nuevo `create` o `load` seguido de `start` la reanuda.
- **Carga atómica:** `load` valida el archivo completo antes de registrar procesos; ante cualquier error no se carga nada.
- **Nombres repetidos:** se permiten; la identidad de un proceso es su PID.

## Pruebas y evidencia

```bash
make test        # pruebas de tests/test_*.cpp (release) y tests/prueba_consola.sh
make test-tsan   # lo mismo con ThreadSanitizer
```

`make test` ejecuta todas las pruebas sin detenerse en el primer fallo, imprime `OK <nombre>` o `FALLO <nombre>`, un resumen y termina con código distinto de 0 si alguna falló. `prueba_consola.sh` compila el simulador, ejecuta `tests/entradas/consola_basica.txt` y compara la salida con `tests/esperado/consola_basica.txt`. **Problema conocido:** esa prueba falla hoy porque `tests/esperado/consola_basica.txt` está guardado en UTF-16 y el `diff` contra la salida del simulador nunca coincide; las 12 pruebas restantes pasan.

Para generar la evidencia de concurrencia y memoria (TSan, ASan/UBSan, Valgrind memcheck y Helgrind sobre el simulador real) y dejarla en `docs/evidencia/` junto con un `RESUMEN.md`:

```bash
bash scripts/generar_evidencia.sh   # tarda varios minutos; ver la cabecera del script
```

La carpeta `docs/evidencia/` aún no está incluida en el repositorio. <!-- COMPLETAR: enlazar docs/evidencia/RESUMEN.md cuando se genere y se agregue -->

## Equipo y quién hizo qué

Los nombres salen del historial de git; los datos que faltan quedan marcados.

| Autor (git) | Nombre completo / carnet | Trabajo principal según el historial |
|---|---|---|
| `vsbanezca` | <!-- COMPLETAR --> | `EstadoSimulador`, `Cerrojo`, `TablaProcesos`, `ColaListos`, `ListaLlegadas`, `GestorProcesos` y transiciones del PCB (issues #4, #6, #8); comando `ps` (#17); `main` (#18); `Motor`, políticas RR/SRTF, `RegistroGantt` y `Metricas` (#31); comandos de control y reportes (#32); `load`, parser de trazas y `workloads/` (#35); `Makefile` (#40); script de evidencia y sincronización de la salida (#41); este README (#43) |
| Anabella Freites | <!-- COMPLETAR --> | `Mensajes`, `ValidadorNombre`, `Tokenizador`, `ParserEntero`, bucle de lectura del CLI y tabla de comandos con `ContextoCli` y `exit` (#15) |
| Jose Quintero | <!-- COMPLETAR --> | Estructura inicial y `PCB`, comando `create` (#16), script de prueba de consola (#19) |
| Vanessa Sarai Banezca Luces | <!-- COMPLETAR --> | Aparece en el historial como autora de merges de pull requests (otra identidad de git; <!-- COMPLETAR: confirmar si es la misma persona que `vsbanezca` --> ) |

<!-- COMPLETAR: integrantes del equipo que no aparezcan en el historial de git -->

## Uso de inteligencia artificial

El uso de herramientas de inteligencia artificial en este proyecto se documenta en el anexo del informe técnico.
<!-- COMPLETAR: confirmar la referencia exacta al anexo del informe -->
