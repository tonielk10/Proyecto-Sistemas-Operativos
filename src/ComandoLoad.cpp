#include "../include/Comandos.h"
#include "../include/Mensajes.h"
#include "../include/Traza.h"

#include <sys/stat.h>
#include <fstream>
#include <iostream>

namespace {
constexpr long long TAMANO_MAX_BYTES = 10LL * 1024 * 1024;
constexpr std::size_t ERRORES_MOSTRADOS = 20;
}

void comandoLoad(const std::vector<std::string>& args, ContextoCli& ctx) {
    if (args.empty()) {
        std::cout << Mensajes::errorSintaxis("Parámetros insuficientes.", Mensajes::USO_LOAD) << "\n";
        return;
    }
    if (args.size() > 1) {
        std::cout << Mensajes::errorSintaxis("Demasiados parámetros.", Mensajes::USO_LOAD) << "\n";
        return;
    }

    const std::string& ruta = args[0];
    struct stat info;
    if (stat(ruta.c_str(), &info) != 0) {
        std::cout << Mensajes::errorParametro("El archivo '" + ruta + "' no existe.") << "\n";
        return;
    }
    if (!S_ISREG(info.st_mode)) {
        std::cout << Mensajes::errorParametro("'" + ruta + "' no es un archivo regular.") << "\n";
        return;
    }
    if (static_cast<long long>(info.st_size) > TAMANO_MAX_BYTES) {
        std::cout << Mensajes::errorParametro("El archivo supera el tamaño máximo (10 MB).") << "\n";
        return;
    }
    std::ifstream archivo(ruta);
    if (!archivo) {
        std::cout << Mensajes::errorParametro("No se puede leer '" + ruta + "' (revisa los permisos).") << "\n";
        return;
    }

    ResultadoTraza traza = parsearTraza(archivo);
    if (!traza.errores.empty()) {
        std::size_t total = traza.errores.size();
        for (std::size_t i = 0; i < total && i < ERRORES_MOSTRADOS; ++i) {
            std::cout << Mensajes::errorGeneral("Línea " + std::to_string(traza.errores[i].linea) + ": " +
                                                traza.errores[i].detalle) << "\n";
        }
        if (total > ERRORES_MOSTRADOS) {
            std::cout << "... y " << (total - ERRORES_MOSTRADOS) << " errores más.\n";
        }
        std::cout << Mensajes::errorGeneral("El archivo '" + ruta + "' tiene " + std::to_string(total) +
                                            " línea(s) inválida(s): no se cargó ningún proceso.") << "\n";
        return;
    }
    if (traza.entradas.empty()) {
        std::cout << Mensajes::mensajeInfo("El archivo '" + ruta + "' no contiene procesos.") << "\n";
        return;
    }

    // ARRIVAL_TIME del archivo es relativo al reloj actual: llegada real = reloj + ARRIVAL_TIME.
    int base = ctx.estado.leer_reloj();
    int primero = 0, ultimo = 0;
    for (std::size_t i = 0; i < traza.entradas.size(); ++i) {
        const EntradaTraza& e = traza.entradas[i];
        int pid = ctx.gestor.registrar_llegada_futura(e.nombre, e.burst, e.prioridad, base + e.llegada);
        if (pid == -1) {
            std::cout << Mensajes::errorGeneral("No se pudo registrar el proceso '" + e.nombre + "'.") << "\n";
            return;
        }
        if (i == 0) primero = pid;
        ultimo = pid;
    }

    std::cout << Mensajes::mensajeOk("Se cargaron " + std::to_string(traza.entradas.size()) + " proceso(s) desde '" +
                                     ruta + "' (PID " + std::to_string(primero) + " a " + std::to_string(ultimo) + ").") << "\n";
    if (ctx.estado.esta_pausado()) {
        std::cout << Mensajes::mensajeInfo("Usa 'start' para iniciar la simulación.") << "\n";
    }
}
