#include "../include/Mensajes.h"
#include <iostream>
#include <string>
#include <vector>

// Función auxiliar para comprobar y reportar errores
bool comprobar(const std::string& nombre_prueba, const std::string& obtenido, const std::string& esperado) {
    if (obtenido != esperado) {
        std::cerr << "[FALLO] " << nombre_prueba << "\n"
                  << "  Esperaba: " << esperado << "\n"
                  << "  Obtuvo:   " << obtenido << "\n";
        return false;
    }
    return true;
}

int main() {
    bool todo_ok = true;

    // 1. Verificar las 12 constantes de sintaxis
    todo_ok &= comprobar("USO_CREATE", Mensajes::USO_CREATE, "create <nombre> <burst_time> [prioridad]");
    todo_ok &= comprobar("USO_LOAD", Mensajes::USO_LOAD, "load <archivo>");
    todo_ok &= comprobar("USO_PS", Mensajes::USO_PS, "ps");
    todo_ok &= comprobar("USO_PCB", Mensajes::USO_PCB, "pcb <PID>");
    todo_ok &= comprobar("USO_SET_SCHED", Mensajes::USO_SET_SCHED, "set_sched <algoritmo> [quantum]");
    todo_ok &= comprobar("USO_SET_SPEED", Mensajes::USO_SET_SPEED, "set_speed <ms_por_tick>");
    todo_ok &= comprobar("USO_START", Mensajes::USO_START, "start");
    todo_ok &= comprobar("USO_PAUSE", Mensajes::USO_PAUSE, "pause");
    todo_ok &= comprobar("USO_GANTT", Mensajes::USO_GANTT, "gantt");
    todo_ok &= comprobar("USO_STATS", Mensajes::USO_STATS, "stats");
    todo_ok &= comprobar("USO_HELP", Mensajes::USO_HELP, "help");
    todo_ok &= comprobar("USO_EXIT", Mensajes::USO_EXIT, "exit");

    // 2. Verificar las 6 funciones con los casos exactos de la tabla del Kanban
    
    std::string resSintaxis = Mensajes::errorSintaxis("Parámetros insuficientes.", Mensajes::USO_CREATE);
    std::string expSintaxis = "[ERROR DE SINTAXIS]: Parámetros insuficientes.\nUso correcto: create <nombre> <burst_time> [prioridad]";
    todo_ok &= comprobar("errorSintaxis", resSintaxis, expSintaxis);

    std::string resTipo = Mensajes::errorTipo("El PID debe ser un número entero positivo.");
    std::string expTipo = "[ERROR DE TIPO]: El PID debe ser un número entero positivo.";
    todo_ok &= comprobar("errorTipo", resTipo, expTipo);

    std::string resParam = Mensajes::errorParametro("Algoritmo no reconocido. Algoritmos válidos: RR, SRTF.");
    std::string expParam = "[ERROR DE PARAMETRO]: Algoritmo no reconocido. Algoritmos válidos: RR, SRTF.";
    todo_ok &= comprobar("errorParametro", resParam, expParam);

    std::string resGen = Mensajes::errorGeneral("Comando no reconocido. Escribe 'help'");
    std::string expGen = "[ERROR]: Comando no reconocido. Escribe 'help'";
    todo_ok &= comprobar("errorGeneral", resGen, expGen);

    std::string resOk = Mensajes::mensajeOk("Proceso 'P1' creado con PID 1");
    std::string expOk = "[OK] Proceso 'P1' creado con PID 1";
    todo_ok &= comprobar("mensajeOk", resOk, expOk);

    std::string resInfo = Mensajes::mensajeInfo("No hay procesos en el sistema.");
    std::string expInfo = "[INFO] No hay procesos en el sistema.";
    todo_ok &= comprobar("mensajeInfo", resInfo, expInfo);

    if (!todo_ok) {
        std::cerr << "\nAl menos una prueba fallo. Revisa la salida de arriba.\n";
        return 1;
    }

    std::cout << "Exito: Todas las 12 constantes y 6 funciones de mensajes pasaron correctamente.\n";
    return 0;
}