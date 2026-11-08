// ColaListos.h - Cola FIFO de procesos en estado LISTO, compartida entre hilos.
// Dependencias: <pthread.h>, <deque>, <vector>
//
// Guarda solo PIDs: los datos del proceso viven unicamente en TablaProcesos, asi no hay
// dos copias del mismo PCB que puedan desincronizarse.
// Sigue la regla de locking descrita en TablaProcesos.h.
#ifndef COLA_LISTOS_H
#define COLA_LISTOS_H

#include <pthread.h>
#include <cstddef>
#include <deque>
#include <vector>

class ColaListos {
public:
    ColaListos();
    ~ColaListos();

    ColaListos(const ColaListos&) = delete;
    ColaListos& operator=(const ColaListos&) = delete;

    /** Toma el mutex. Agrega al final; false si pid <= 0 o ya esta en la cola. */
    bool encolar(int pid);
    /** Toma el mutex. Saca el primero en pid; false si la cola esta vacia. */
    bool desencolar(int& pid);
    /** Toma el mutex. Copia el primero en pid sin sacarlo; false si esta vacia. */
    bool ver_primero(int& pid) const;
    /** Toma el mutex. Quita pid de cualquier posicion; false si no estaba. */
    bool quitar(int pid);
    /** Toma el mutex. */
    bool contiene(int pid) const;
    /** Toma el mutex. Foto de los PIDs en orden de la cola. */
    std::vector<int> listar() const;
    /** Toma el mutex. */
    std::size_t tamano() const;
    /** Toma el mutex. */
    bool vacia() const;

private:
    mutable pthread_mutex_t mutex_;
    std::deque<int> cola_;
};

#endif
