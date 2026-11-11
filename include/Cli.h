#ifndef CLI_H
#define CLI_H

#include <string>
#include <map>
#include "ContextoCli.h"
#include "Comandos.h"

class Cli {
public:
    //constructor explicit que recibe el contexto
    explicit Cli(ContextoCli& ctx);
    void ejecutar();

private:
    void procesarLinea(const std::string& linea);
    
    ContextoCli& ctx_; // Referencia al contexto global
    std::map<std::string, ManejadorComando> comandos_; // Tabla de enrutamiento
};

#endif // CLI_H