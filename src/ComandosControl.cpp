// Comandos de control de la simulacion: start, pause, help, set_sched y set_speed.

#include "../include/Comandos.h"
#include "../include/Mensajes.h"
#include "../include/ParserEntero.h"
#include <iostream>
#include <string>

namespace {

bool sinParametros(const std::vector<std::string>& args, const std::string& uso) {
    if (args.empty()) return true;
    std::cout << Mensajes::errorSintaxis("Este comando no recibe parámetros.", uso) << "\n";
    return false;
}

} // namespace

void comandoStart(const std::vector<std::string>& args, ContextoCli& ctx) {
    if (!sinParametros(args, Mensajes::USO_START)) return;
    if (ctx.estado.esta_pausado()) {
        ctx.estado.iniciar();
        std::cout << Mensajes::mensajeOk("Simulación iniciada.") << "\n";
    } else {
        std::cout << Mensajes::mensajeInfo("La simulación ya está en ejecución.") << "\n";
    }
}

void comandoPause(const std::vector<std::string>& args, ContextoCli& ctx) {
    if (!sinParametros(args, Mensajes::USO_PAUSE)) return;
    if (!ctx.estado.esta_pausado()) {
        ctx.estado.pausar();
        std::cout << Mensajes::mensajeOk("Simulación en pausa (termina el tick actual).") << "\n";
    } else {
        std::cout << Mensajes::mensajeInfo("La simulación ya está en pausa.") << "\n";
    }
}

void comandoHelp(const std::vector<std::string>& args, ContextoCli&) {
    if (!sinParametros(args, Mensajes::USO_HELP)) return;
    using namespace Mensajes;
    // load no esta registrado aun y ContextoCli no expone la tabla de comandos
    std::cout << "COMANDOS DISPONIBLES:\n"
              << "\n" << USO_CREATE << "\n"
              << "    Crea un proceso y lo deja listo para ejecutar.\n"
              << "    Prioridad 1-5 (1 = más alta), por defecto 3. Ejemplo: create P1 5 2\n"
              << "\n" << USO_LOAD << " (próximamente)\n"
              << "    Carga procesos desde un archivo. Ejemplo: load procesos.txt\n"
              << "\n" << USO_PS << "\n"
              << "    Lista todos los procesos con su estado.\n"
              << "\n" << USO_PCB << "\n"
              << "    Muestra el bloque de control de un proceso. Ejemplo: pcb 1\n"
              << "\n" << USO_SET_SCHED << "\n"
              << "    Elige la política: RR (en mayúsculas) o SRTF. Quantum entero 1-1000, por defecto 2\n"
              << "    (solo aplica a RR). Ejemplo: set_sched RR 3\n"
              << "\n" << USO_SET_SPEED << "\n"
              << "    Duración de cada tick en milisegundos, de 50 a 5000, por defecto 1000.\n"
              << "    Ejemplo: set_speed 200\n"
              << "\n" << USO_START << "\n"
              << "    Inicia o reanuda la simulación.\n"
              << "\n" << USO_PAUSE << "\n"
              << "    Pausa la simulación al terminar el tick actual.\n"
              << "\n" << USO_GANTT << "\n"
              << "    Muestra el diagrama de Gantt de lo ejecutado hasta ahora.\n"
              << "\n" << USO_STATS << "\n"
              << "    Muestra espera, retorno y respuesta promedio, y uso de CPU.\n"
              << "\n" << USO_HELP << "\n"
              << "    Muestra esta ayuda.\n"
              << "\n" << USO_EXIT << "\n"
              << "    Cierra el simulador.\n";
}

void comandoSetSched(const std::vector<std::string>& args, ContextoCli& ctx) {
    if (args.empty()) {
        std::cout << Mensajes::errorSintaxis("Parámetros insuficientes.", Mensajes::USO_SET_SCHED) << "\n";
        return;
    }
    if (args.size() > 2) {
        std::cout << Mensajes::errorSintaxis("Demasiados parámetros.", Mensajes::USO_SET_SCHED) << "\n";
        return;
    }
    const std::string& algoritmo = args[0];
    if (algoritmo != "RR" && algoritmo != "SRTF") {
        std::cout << Mensajes::errorParametro("Algoritmo no reconocido. Algoritmos válidos: RR, SRTF.") << "\n";
        return;
    }
    long q = 2;
    bool hay_quantum = args.size() == 2;
    if (hay_quantum) {
        ResultadoParseo r = parsearEntero(args[1], 1, 1000, q);
        if (r == ResultadoParseo::NO_ENTERO) {
            std::cout << Mensajes::errorTipo("El quantum debe ser un número entero.") << "\n";
            return;
        }
        if (r == ResultadoParseo::FUERA_DE_RANGO) {
            std::cout << Mensajes::errorParametro("El quantum debe estar entre 1 y 1000.") << "\n";
            return;
        }
    }
    if (algoritmo == "RR") {
        ctx.estado.set_politica(Politica::RR, static_cast<int>(q));
        std::cout << Mensajes::mensajeOk("Política: RR (quantum=" + std::to_string(q) + ")") << "\n";
    } else {
        if (hay_quantum) std::cout << Mensajes::mensajeInfo("El quantum no aplica a SRTF y se ignora.") << "\n";
        ctx.estado.set_politica(Politica::SRTF, ctx.estado.leer_config().quantum);
        std::cout << Mensajes::mensajeOk("Política: SRTF") << "\n";
    }
}

void comandoSetSpeed(const std::vector<std::string>& args, ContextoCli& ctx) {
    if (args.empty()) {
        std::cout << Mensajes::errorSintaxis("Parámetros insuficientes.", Mensajes::USO_SET_SPEED) << "\n";
        return;
    }
    if (args.size() > 1) {
        std::cout << Mensajes::errorSintaxis("Demasiados parámetros.", Mensajes::USO_SET_SPEED) << "\n";
        return;
    }
    long ms = 0;
    ResultadoParseo r = parsearEntero(args[0], 50, 5000, ms);
    if (r == ResultadoParseo::NO_ENTERO) {
        std::cout << Mensajes::errorTipo("La velocidad debe ser un número entero en milisegundos.") << "\n";
        return;
    }
    if (r == ResultadoParseo::FUERA_DE_RANGO) {
        std::cout << Mensajes::errorParametro("La velocidad debe estar entre 50 y 5000 ms.") << "\n";
        return;
    }
    ctx.estado.set_velocidad(static_cast<int>(ms));
    std::cout << Mensajes::mensajeOk("Velocidad: " + std::to_string(ms) + " ms/tick") << "\n";
}
