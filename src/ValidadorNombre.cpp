#include "../include/ValidadorNombre.h"

bool nombreValido(const std::string& nombre) {
    // Tiene entre 1 y 32 caracteres (bytes).
    if (nombre.empty() || nombre.length() > NOMBRE_MAX) {
        return false;
    }

    for (char c_signed : nombre) {
        // Convertir a unsigned char para evitar que las letras con tilde 
        // o la 'ñ' (que en UTF-8 usan el bit más significativo) se evalúen como negativas.
        unsigned char c = static_cast<unsigned char>(c_signed);
        
        // No contiene espacios, tabs ni caracteres de control (0 a 32, y 127), tampoco contiene comas.
        if (c <= 32 || c == 127 || c == ',') {
            return false;
        }
    }
    
    return true;
}