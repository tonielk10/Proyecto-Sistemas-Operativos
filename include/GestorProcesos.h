// GestorProcesos.h - Crea procesos, asigna PIDs y centraliza sus cambios de estado.
// Dependencias: PCB.h, EstadoSimulador.h, TablaProcesos.h, ColaListos.h, ListaLlegadas.h
//
// El gestor no es dueno de las estructuras: recibe referencias y deben vivir mas que el.
// Su mutex protege solo el contador de PID. Siguiendo la regla de locking de
// TablaProcesos.h, se suelta antes de llamar a cualquier otra estructura, asi que
// nunca hay dos locks tomados a la vez.
//
// Se permiten nombres de proceso repetidos: la identidad la da el PID, que es unico.
#ifndef GESTOR_PROCESOS_H
#define GESTOR_PROCESOS_H

#include <pthread.h>
#include <string>

#include "ColaListos.h"
#include "EstadoSimulador.h"
#include "ListaLlegadas.h"
#include "PCB.h"
#include "TablaProcesos.h"

class GestorProcesos {
public:
    GestorProcesos(EstadoSimulador& estado, TablaProcesos& tabla,
                   ColaListos& cola, ListaLlegadas& llegadas);
    ~GestorProcesos();

    GestorProcesos(const GestorProcesos&) = delete;
    GestorProcesos& operator=(const GestorProcesos&) = delete;

    /**
     * Crea un proceso que llega ahora (reloj actual) y lo deja READY en la cola.
     * Devuelve el PID (>= 1) o -1 si los datos son invalidos, sin consumir PID.
     */
    int crear_proceso(const std::string& nombre, int burst, int prioridad = 3);
    /**
     * Registra un proceso en NEW que llegara en el tick llegada: queda en la tabla y
     * en la lista de llegadas, no en la cola. Devuelve el PID o -1 sin consumir PID.
     */
    int registrar_llegada_futura(const std::string& nombre, int burst, int prioridad, int llegada);
    /** Pasa el proceso de NEW a READY y lo encola; false si no existe, no esta en NEW o ya esta en la cola. */
    bool pasar_a_listo(int pid);
    /** Aplica PCB::cambiar_estado sobre el proceso; false si el PID no existe o la transicion es invalida. */
    bool cambiar_estado(int pid, EstadoProceso nuevo);
    /** Toma el mutex del gestor. Ultimo PID asignado; 0 si aun no se creo ninguno. */
    int ultimo_pid() const;

private:
    static bool datos_validos(const std::string& nombre, int burst, int prioridad);
    int siguiente_pid();

    mutable pthread_mutex_t mutex_;
    int ultimo_pid_;

    EstadoSimulador& estado_;
    TablaProcesos& tabla_;
    ColaListos& cola_;
    ListaLlegadas& llegadas_;
};

#endif
