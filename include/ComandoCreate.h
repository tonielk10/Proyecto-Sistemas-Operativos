#ifndef COMANDO_CREATE_H
#define COMANDO_CREATE_H

#include "ContextoCli.h"
#include <vector>
#include <string>

/**
 * @brief Manejador para el comando create <nombre> <burst> [prioridad].
 * Valida sintaxis, tipos y restricciones del proceso antes de crearlo.
 */
void comandoCreate(const std::vector<std::string>& args, ContextoCli& ctx);

#endif // COMANDO_CREATE_H