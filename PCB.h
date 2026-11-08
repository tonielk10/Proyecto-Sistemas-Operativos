#ifndef PCB_H 
#define PCB_H

#include <cstdio>
#include <string>

enum class EstadoProceso{ //clase enumerada para definir los estados de un proceso
    NEW, 
    READY, 
    RUNNING, 
    WAITING, 
    TERMINATED
};

inline const char* nombreEstado(EstadoProceso num){             //funcion auxiliar para convertir el estado del proceso a string
    switch (num) {
        case EstadoProceso::NEW:      return "NEW";
        case EstadoProceso::READY:      return "READY";
        case EstadoProceso::RUNNING:  return "RUNNING";
        case EstadoProceso::WAITING:    return "WAITING";
        case EstadoProceso::TERMINATED: return "TERMINATED";
    }
    return "DESCONOCIDO";
}

// Transiciones permitidas del ciclo de vida; cualquier otra (incluida al mismo estado) es invalida.
inline bool transicionValida(EstadoProceso desde, EstadoProceso hasta){
    switch (desde) {
        case EstadoProceso::NEW:     return hasta == EstadoProceso::READY;
        case EstadoProceso::READY:   return hasta == EstadoProceso::RUNNING;
        case EstadoProceso::RUNNING: return hasta == EstadoProceso::READY ||
                                            hasta == EstadoProceso::WAITING ||
                                            hasta == EstadoProceso::TERMINATED;
        case EstadoProceso::WAITING: return hasta == EstadoProceso::READY;
        case EstadoProceso::TERMINATED: return false;
    }
    return false;
}

class PCB{
    public:
    //identificacion de cada proceso
        int PID;                   //Identificador unico del proceso
        std::string nombre;        //Nombre del proceso
        EstadoProceso estado;      //Estado actual del proceso

    //Parametros de planificación del proceso
        int tiempo_llegada;        //momento en que el proceso llego al sistema
        int tiempo_rafaga;         //tiempo de rafaga del proceso
        int tiempo_restante;       //tiempo restante de ejecucion
        int prioridad;             //nivel de prioridad del proceso

    //metricas de rendimiento del proceso
        int tiempo_respuesta;      //momento en que el proceso inicia su ejecucion
        int tiempo_retorno;        //momento en que el proceso finaliza su ejecucion
        int tiempo_espera;         //tiempo total en la cola de listos

    //constructor
    PCB(int PID, std::string nombre, int tiempo_llegada, int tiempo_rafaga, int prioridad)
        : PID(PID),
        nombre(nombre),
        estado(EstadoProceso::NEW),
        tiempo_llegada(tiempo_llegada),
        tiempo_rafaga(tiempo_rafaga), 
        tiempo_restante(tiempo_rafaga), 
        prioridad(prioridad), 
        tiempo_respuesta(-1), 
        tiempo_retorno(-1), 
        tiempo_espera(0) {}

    /** Cambia a nuevo si transicionValida lo permite; si no, no toca nada, avisa por stderr y devuelve false. */
    bool cambiar_estado(EstadoProceso nuevo){
        if (!transicionValida(estado, nuevo)) {
            std::fprintf(stderr, "[ERROR INTERNO] Transicion invalida PID %d: %s -> %s\n",
                         PID, nombreEstado(estado), nombreEstado(nuevo));
            return false;
        }
        estado = nuevo;
        return true;
    }

};

#endif