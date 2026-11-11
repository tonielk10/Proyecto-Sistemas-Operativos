// Salida.h - Cerrojo unico para escribir en std::cout desde el hilo CLI y el hilo Motor.
// Dependencias: <pthread.h>
#ifndef SALIDA_H
#define SALIDA_H

#include <pthread.h>

// Orden de cerrojos: este se toma primero (CLI) o sin otros cerrojos tomados (Motor).
inline pthread_mutex_t mutex_salida = PTHREAD_MUTEX_INITIALIZER;

#endif
