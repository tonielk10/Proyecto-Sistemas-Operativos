// Comando ps: lista la tabla de procesos con el formato del enunciado.

#include "../include/Comandos.h"
#include "../include/Mensajes.h"
#include <iostream>
#include <string>

namespace {

// Rellena con espacios hasta el ancho; si el texto ya lo alcanza, no lo corta y deja un espacio
std::string columna(const std::string& texto, std::size_t ancho) {
    if (texto.size() >= ancho) return texto + " ";
    return texto + std::string(ancho - texto.size(), ' ');
}

} // namespace

void comandoPs(const std::vector<std::string>& args, ContextoCli& ctx) {
    if (!args.empty()) {
        std::cout << Mensajes::errorSintaxis("Este comando no recibe parámetros.", Mensajes::USO_PS) << "\n";
        return;
    }

    auto procesos = ctx.tabla.listar();
    if (procesos.empty()) {
        std::cout << Mensajes::mensajeInfo("No hay procesos en el sistema.") << "\n";
        return;
    }

    std::cout << columna("PID", 7) << columna("NOMBRE", 15) << columna("ESTADO", 13)
              << columna("BURST RESTANTE", 18) << "PRIORIDAD\n";
    std::cout << std::string(60, '-') << "\n";

    for (const PCB& p : procesos) {
        std::cout << columna(std::to_string(p.PID), 7) << columna(p.nombre, 15)
                  << columna(nombreEstado(p.estado), 13)
                  << columna(std::to_string(p.tiempo_restante) + "s", 18)
                  << p.prioridad << "\n";
    }
}
