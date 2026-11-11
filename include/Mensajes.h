// mensajes de error y de ayuda del sistema se encuentran en este archivo, para que sea más fácil
// mantenerlos y modificarlos.

#ifndef MENSAJES_H
#define MENSAJES_H

#include <string>

namespace Mensajes {

    // Constantes con la sintaxis de cada comando (Sección 4 del enunciado)
    inline const std::string USO_CREATE    = "create <nombre> <burst_time> [prioridad]";
    inline const std::string USO_LOAD      = "load <archivo>";
    inline const std::string USO_PS        = "ps";
    inline const std::string USO_PCB       = "pcb <PID>";
    inline const std::string USO_SET_SCHED = "set_sched <algoritmo> [quantum]";
    inline const std::string USO_SET_SPEED = "set_speed <ms_por_tick>";
    inline const std::string USO_START     = "start";
    inline const std::string USO_PAUSE     = "pause";
    inline const std::string USO_GANTT     = "gantt";
    inline const std::string USO_STATS     = "stats";
    inline const std::string USO_HELP      = "help";
    inline const std::string USO_EXIT      = "exit";

    // Funciones de formateo de errores (devuelven el texto sin salto de línea al final)
    inline std::string errorSintaxis(const std::string& detalle, const std::string& uso) {
        return "[ERROR DE SINTAXIS]: " + detalle + "\nUso correcto: " + uso;
    }

    inline std::string errorTipo(const std::string& detalle) {
        return "[ERROR DE TIPO]: " + detalle;
    }

    inline std::string errorParametro(const std::string& detalle) {
        return "[ERROR DE PARAMETRO]: " + detalle;
    }

    inline std::string errorGeneral(const std::string& detalle) {
        return "[ERROR]: " + detalle;
    }

    inline std::string mensajeOk(const std::string& detalle) {
        return "[OK] " + detalle;
    }

    inline std::string mensajeInfo(const std::string& detalle) {
        return "[INFO] " + detalle;
    }

} // namespace Mensajes

#endif // MENSAJES_H