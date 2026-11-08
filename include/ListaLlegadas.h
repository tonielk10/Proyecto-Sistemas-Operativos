// ListaLlegadas.h - Procesos que aun no llegan al sistema, ordenados por tiempo de llegada.
// Dependencias: <pthread.h>, <set>, <utility>, <vector>
//
// Guarda pares (llegada, pid): el set los ordena por llegada y, a igual llegada,
// por PID, lo que hace determinista el orden en que pasan a LISTOS.
// Sigue la regla de locking descrita en TablaProcesos.h.
#ifndef LISTA_LLEGADAS_H
#define LISTA_LLEGADAS_H

#include <pthread.h>
#include <cstddef>
#include <set>
#include <utility>
#include <vector>

class ListaLlegadas {
public:
    ListaLlegadas();
    ~ListaLlegadas();

    ListaLlegadas(const ListaLlegadas&) = delete;
    ListaLlegadas& operator=(const ListaLlegadas&) = delete;

    /** Toma el mutex. false si llegada < 0, pid <= 0 o el pid ya esta en la lista. */
    bool agregar(int llegada, int pid);
    /** Toma el mutex. Saca y devuelve, en orden, los PIDs con llegada <= reloj. */
    std::vector<int> extraer_hasta(int reloj);
    /** Toma el mutex. */
    bool hay_pendientes() const;
    /** Toma el mutex. */
    std::size_t tamano() const;

private:
    mutable pthread_mutex_t mutex_;
    std::set<std::pair<int, int>> llegadas_;
    std::set<int> pids_;  // indice para detectar duplicados sin recorrer llegadas_
};

#endif
