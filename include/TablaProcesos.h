// TablaProcesos.h - Tabla de procesos compartida: unico lugar donde viven los PCB.
// Dependencias: PCB.h, <pthread.h>, <map>, <vector>, <functional>
//
// Regla de locking (aplica tambien a ColaListos y ListaLlegadas): ningun metodo llama
// a otra estructura mientras tiene su propio mutex tomado. Cada metodo toma un solo
// lock y lo suelta al salir, asi nunca hay locks anidados y no puede haber deadlock
// por orden de adquisicion. Quien coordine varias estructuras (el motor) debe hacerlo
// en pasos separados, sin mantener un lock entre llamadas.
#ifndef TABLA_PROCESOS_H
#define TABLA_PROCESOS_H

#include <pthread.h>
#include <cstddef>
#include <functional>
#include <map>
#include <vector>

#include "PCB.h"

class TablaProcesos {
public:
    TablaProcesos();
    ~TablaProcesos();

    TablaProcesos(const TablaProcesos&) = delete;
    TablaProcesos& operator=(const TablaProcesos&) = delete;

    /** Toma el mutex. Inserta una copia de p; false si p.PID <= 0 o el PID ya existe. */
    bool agregar(const PCB& p);
    /** Toma el mutex. Copia el PCB en salida; false (salida sin tocar) si no existe. */
    bool obtener(int pid, PCB& salida) const;
    /** Toma el mutex. */
    bool existe(int pid) const;
    /**
     * Toma el mutex y aplica fn al PCB; false si no existe o si fn cambia el PID.
     * fn corre con el mutex tomado: no debe llamar a otras estructuras ni bloquearse.
     */
    bool actualizar(int pid, const std::function<void(PCB&)>& fn);
    /** Toma el mutex. Foto de todos los PCB ordenada por PID. */
    std::vector<PCB> listar() const;
    /** Toma el mutex. */
    std::size_t tamano() const;

private:
    mutable pthread_mutex_t mutex_;
    std::map<int, PCB> procesos_;
};

#endif
