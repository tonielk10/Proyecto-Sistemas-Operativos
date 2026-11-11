#include "../include/Cli.h"
#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>

void Cli::ejecutar() {
    std::string linea;

    while (!salir_) {
        // Regla 1: Muestra el prompt y vacía el buffer
        std::cout << "> " << std::flush;

        // Regla 2 y 3: Lee la entrada con getline y maneja fallos (Ctrl-D / EOF)
        if (!std::getline(std::cin, linea)) {
            std::cout << "\n";
            break; // Termina igual que exit
        }

        // Regla 4: Quita un \r al final si lo hay (CRLF de Windows)
        if (!linea.empty() && linea.back() == '\r') {
            linea.pop_back();
        }

        // Regla 5: Si la línea está vacía o solo tiene espacios o tabs, no hace nada
        bool solo_espacios = true;
        for (char c : linea) {
            if (c != ' ' && c != '\t') {
                solo_espacios = false;
                break;
            }
        }
        if (solo_espacios) {
            continue;
        }

        // Regla 6: Límite estricto de 1024 caracteres para prevenir desbordamientos
        if (linea.length() > 1024) {
            std::cout << "[ERROR]: Linea demasiado larga (maximo 1024 caracteres).\n";
            continue;
        }

        // Regla 7: Rechazar caracteres de control (bytes < 32 excepto tab, o 127)
        bool caracteres_invalidos = false;
        for (char c_signed : linea) {
            unsigned char c = static_cast<unsigned char>(c_signed);
            if ((c < 32 && c != '\t') || c == 127) {
                caracteres_invalidos = true;
                break;
            }
        }
        
        if (caracteres_invalidos) {
            std::cout << "[ERROR]: La linea contiene caracteres no validos.\n";
            continue;
        }

        // Regla 8: Pasar la línea limpia al procesador
        procesarLinea(linea);
    }
}

void Cli::procesarLinea(const std::string& linea) {
    // Procesamiento provisional (Ticket #14)
    std::string comando = linea;

    // Quitar espacios al inicio y al final
    comando.erase(comando.begin(), std::find_if(comando.begin(), comando.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
    comando.erase(std::find_if(comando.rbegin(), comando.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), comando.end());

    if (comando == "exit") {
        salir_ = true;
    } else {
        std::cout << "[ERROR]: Comando no reconocido. Escribe 'help'.\n";
    }
}