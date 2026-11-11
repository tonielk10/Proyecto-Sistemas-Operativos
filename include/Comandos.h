//firma de los comandos del sistema, para que el CLI pueda invocarlos de manera uniforme.

#ifndef COMANDOS_H
#define COMANDOS_H

#include "ContextoCli.h"
#include <vector>
#include <string>

// Puntero a función que define cómo debe ser cualquier comando del sistema
using ManejadorComando = void (*)(const std::vector<std::string>& args, ContextoCli& ctx);

// Declaración del comando exit
void comandoExit(const std::vector<std::string>& args, ContextoCli& ctx);

#endif // COMANDOS_H