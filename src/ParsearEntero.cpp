#include "ParserEntero.h"
#include <cstdlib>
#include <cerrno>
#include <cctype>

ResultadoParseo parsearEntero(const std::string& texto, long minimo, long maximo, long& salida) {
    if (texto.empty()) {
        return ResultadoParseo::NO_ENTERO;
    }

    size_t inicio = 0;

    // Validar el primer carácter para rechazar '+' o espacios
    if (texto[0] == '-') {
        if (texto.length() == 1) return ResultadoParseo::NO_ENTERO; // Rechazar solo "-"
        inicio = 1;
    } else if (!std::isdigit(static_cast<unsigned char>(texto[0]))) {
        return ResultadoParseo::NO_ENTERO;
    }

    // Validar que el resto de la cadena sean estrictamente dígitos
    for (size_t i = inicio; i < texto.length(); ++i) {
        if (!std::isdigit(static_cast<unsigned char>(texto[i]))) {
            return ResultadoParseo::NO_ENTERO;
        }
    }

    char* endptr;
    errno = 0; // Resetear errno antes de llamar a strtol
    
    // Uso estricto de strtol, sin atoi ni stoi
    long valor = std::strtol(texto.c_str(), &endptr, 10);

    // Revisar overflow nativo de strtol o si sobró texto (aunque el for previo ya lo protege)
    if (errno == ERANGE) {
        return ResultadoParseo::FUERA_DE_RANGO;
    }
    if (*endptr != '\0') {
        return ResultadoParseo::NO_ENTERO;
    }

    // Validar el rango manual exigido
    if (valor < minimo || valor > maximo) {
        return ResultadoParseo::FUERA_DE_RANGO;
    }

    salida = valor;
    return ResultadoParseo::OK;
}