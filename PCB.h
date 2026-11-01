#ifndef PCB_H 
#define PCB_H

#include <string>

enum class EstadoProceso{ //clase enumerada para definir los estados de un proceso
    NEW, 
    READY, 
    RUNNING, 
    WAITING, 
    TERMINATED
};

const char* nombreEstado(EstadoProceso num){             //funcion auxiliar para convertir el estado del proceso a string
    switch (num) {
        case EstadoProceso::NEW:      return "NEW";
        case EstadoProceso::READY:      return "READY";
        case EstadoProceso::RUNNING:  return "RUNNING";
        case EstadoProceso::WAITING:    return "WAITING";
        case EstadoProceso::TERMINATED: return "TERMINATED";
    }
    return "DESCONOCIDO";
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
        tiempo_llegada(tiempo_llegada), 
        tiempo_rafaga(tiempo_rafaga), 
        tiempo_restante(tiempo_rafaga), 
        prioridad(prioridad), 
        tiempo_respuesta(-1), 
        tiempo_retorno(-1), 
        tiempo_espera(0) {}

};

#endif