#include "../include/Comandos.h"
#include "../include/Mensajes.h"
#include <iostream>

void comandoExit(const std::vector<std::string>& args, ContextoCli& ctx) {
    if (!args.empty()) {
        // si recibe argumentos, muestra errorSintaxis y no sale
        std::cout << Mensajes::errorSintaxis("Este comando no recibe parametros.", Mensajes::USO_EXIT) << "\n";
    } else {
        //si  no recibe argumentos, cambia el estado a salir
        ctx.salir = true;
    }
}