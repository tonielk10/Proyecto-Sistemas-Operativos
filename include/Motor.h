// Motor.h - Hilo que simula el reloj y planifica los procesos tick a tick.
// Dependencias: EstadoSimulador, TablaProcesos, ColaListos, ListaLlegadas, GestorProcesos, RegistroGantt
//
// pid_actual y quantum_usado son locales al hilo del motor: nadie mas los toca, sin lock.
// Nunca duerme con un lock tomado; cada llamada a una estructura toma y suelta su propio lock.
#ifndef MOTOR_H
#define MOTOR_H

#include <pthread.h>

#include "ColaListos.h"
#include "EstadoSimulador.h"
#include "GestorProcesos.h"
#include "ListaLlegadas.h"
#include "RegistroGantt.h"
#include "TablaProcesos.h"

class Motor {
public:
    Motor(EstadoSimulador& estado, TablaProcesos& tabla, ColaListos& cola,
          ListaLlegadas& llegadas, GestorProcesos& gestor, RegistroGantt& gantt);
    ~Motor();

    Motor(const Motor&) = delete;
    Motor& operator=(const Motor&) = delete;

    /** Lanza el hilo del motor; no hace nada si ya esta corriendo. */
    void iniciar_hilo();
    /** Pide el apagado y une el hilo; no hace nada si no esta corriendo. */
    void detener_y_unir();

private:
    static void* punto_entrada(void* arg);
    void bucle();
    void dormir_ms(int ms);

    EstadoSimulador& estado_;
    TablaProcesos& tabla_;
    ColaListos& cola_;
    ListaLlegadas& llegadas_;
    GestorProcesos& gestor_;
    RegistroGantt& gantt_;

    pthread_t hilo_;
    bool corriendo_;  // solo lo tocan iniciar_hilo y detener_y_unir (hilo que es dueno del Motor)
};

#endif
