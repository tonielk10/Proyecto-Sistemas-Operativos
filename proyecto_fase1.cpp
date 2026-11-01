#include <iostream>
#include "PCB.h"
using namespace std;


int main(){
    //simulacion de la creacion de un proceso con su PCB
    PCB p1(1, "Proceso_Primerito", 0, 3, 1);
    p1.estado = EstadoProceso::NEW;

    cout << "======================================================" << endl;
    cout << "         CONTROL BLOCK  DE PROCESO (PCB) - PID: " << p1.PID << endl;
    cout << "Nombre del proceso: " << p1.nombre << endl;
    cout << "Estado Actual: " << nombreEstado(p1.estado) << endl;
    cout << "Prioridad: " << p1.prioridad << endl;
    cout << "Tiempo de llegada: " << p1.tiempo_llegada << " ticks" << endl;
    cout << "Rafaga inicial: " << p1.tiempo_rafaga << " ticks" << endl;
    cout << "Rafaga restante: " << p1.tiempo_restante << "ticks" << endl;
    cout << "Tiempo de espera: " << p1.tiempo_espera << "ticks" << endl;
    cout << "Tiempo de retorno: " << p1.tiempo_retorno << "ticks" << endl;
    cout << "Tiempo de respuesta: " << p1.tiempo_respuesta << "ticks" << endl;
    cout << "======================================================" << endl;

    return 0;
}