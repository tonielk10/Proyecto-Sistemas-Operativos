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

// Declaración del comando ps
void comandoPs(const std::vector<std::string>& args, ContextoCli& ctx);

//Declaración del comando create
void comandoCreate(const std::vector<std::string>& args, ContextoCli& ctx);

// Declaración del comando load (ComandoLoad.cpp)
void comandoLoad(const std::vector<std::string>& args, ContextoCli& ctx);

// Comandos de control (ComandosControl.cpp)
void comandoStart(const std::vector<std::string>& args, ContextoCli& ctx);
void comandoPause(const std::vector<std::string>& args, ContextoCli& ctx);
void comandoHelp(const std::vector<std::string>& args, ContextoCli& ctx);
void comandoSetSched(const std::vector<std::string>& args, ContextoCli& ctx);
void comandoSetSpeed(const std::vector<std::string>& args, ContextoCli& ctx);

// Comandos de reporte (ComandosReportes.cpp)
void comandoPcb(const std::vector<std::string>& args, ContextoCli& ctx);
void comandoGantt(const std::vector<std::string>& args, ContextoCli& ctx);
void comandoStats(const std::vector<std::string>& args, ContextoCli& ctx);

#endif // COMANDOS_H