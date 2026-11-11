//estado compartido entre el hilo CLI y el hilo Motor, junto con las estructuras de datos que representan
//la tabla de procesos, la cola de listos y la lista de llegadas.

#ifndef CONTEXTOCLI_H
#define CONTEXTOCLI_H

// inclusiones de las demas estructuras de datos que el CLI necesita para operar
#include "EstadoSimulador.h"
#include "TablaProcesos.h"
#include "ColaListos.h"
#include "ListaLlegadas.h"
#include "GestorProcesos.h"
#include "RegistroGantt.h"

struct ContextoCli {
    EstadoSimulador& estado;
    TablaProcesos& tabla;
    ColaListos& cola;
    ListaLlegadas& llegadas;
    GestorProcesos& gestor;
    bool salir = false;   // un comando lo pone en true para cerrar la consola
    RegistroGantt* gantt = nullptr;  // al final y como puntero para no romper inicializaciones existentes
};

#endif // CONTEXTOCLI_H