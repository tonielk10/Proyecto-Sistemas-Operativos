#ifndef VALIDADOR_NOMBRE_H
#define VALIDADOR_NOMBRE_H

#include <string>
#include <cstddef>

constexpr std::size_t NOMBRE_MAX = 32;

/* valida que el nombre de un proceso cumpla con los requisitos del sistema.
(Un nombre válido debe tener entre 1 y 32 bytes, y no puede contener espacios, 
tabulaciones, saltos de línea, caracteres de control ni comas)
el parametro es la cadena de texto a validar.
devuelve true si el nombre es válido, false en caso contrario.
 */
bool nombreValido(const std::string& nombre);

#endif // VALIDADOR_NOMBRE_H