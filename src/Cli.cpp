#include "../include/Cli.h"
#include "../include/Tokenizador.h"
#include "../include/Mensajes.h"
#include "../include/Comandos.h"
#include "../include/Cerrojo.h"
#include "../include/Salida.h"
#include <iostream>
#include <vector>

Cli::Cli(ContextoCli& ctx) : ctx_(ctx) {
    // para agregar un comando: declararlo en Comandos.h y registrarlo en el constructor
    comandos_["exit"] = comandoExit;
    comandos_["create"] = comandoCreate;
    comandos_["ps"] = comandoPs;
    comandos_["load"] = comandoLoad;
    comandos_["start"] = comandoStart;
    comandos_["pause"] = comandoPause;
    comandos_["help"] = comandoHelp;
    comandos_["pcb"] = comandoPcb;
    comandos_["set_sched"] = comandoSetSched;
    comandos_["set_speed"] = comandoSetSpeed;
    comandos_["gantt"] = comandoGantt;
    comandos_["stats"] = comandoStats;
}

void Cli::ejecutar() {
    std::string linea;

    // Regla: El bucle termina cuando ctx.salir es true
    while (!ctx_.salir) {
        {
            Cerrojo c(mutex_salida);
            std::cout << "> " << std::flush;
        }

        if (!std::getline(std::cin, linea)) {
            Cerrojo c(mutex_salida);
            std::cout << "\n";
            break; 
        }

        // la salida de este comando no se mezcla con la del Motor
        Cerrojo salida(mutex_salida);

        if (!linea.empty() && linea.back() == '\r') {
            linea.pop_back();
        }

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

        if (linea.length() > 1024) {
            std::cout << "[ERROR]: Linea demasiado larga (maximo 1024 caracteres).\n";
            continue;
        }

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

        procesarLinea(linea);
    }
}

void Cli::procesarLinea(const std::string& linea) {
    // Regla: procesarLinea usa tokenizar
    std::vector<std::string> tokens = tokenizar(linea);
    
    if (tokens.empty()) return;

    // La primera palabra es el comando y el resto son los args
    std::string nombre_comando = tokens[0];
    std::vector<std::string> args(tokens.begin() + 1, tokens.end());

    // Buscar en la tabla de comandos
    auto it = comandos_.find(nombre_comando);
    if (it != comandos_.end()) {
        // Ejecuta la función asignada al comando, pasándole los argumentos y el contexto
        it->second(args, ctx_);
    } else {
        // Regla: Comando no reconocido (Sensible a mayúsculas, EXIT fallará y caerá aquí)
        std::cout << Mensajes::errorGeneral("Comando no reconocido. Escribe 'help'") << "\n";
    }
}