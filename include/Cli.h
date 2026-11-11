#ifndef CLI_H
#define CLI_H

#include <string>

/*Interfaz de línea de comandos interactiva (CLI).
Maneja la lectura segura de entradas del usuario, validando longitud, caracteres de control y EOF sin colapsar el programa.
 */
class Cli {
public:
    /*Inicia el bucle interactivo de lectura y procesamiento.
    Muestra el prompt, captura entradas, las limpia y despacha.
     */
    void ejecutar();

private:
    /*función provisional de enrutamiento de comandos (Ticket #14).
    Evalúa únicamente el comando 'exit' y notifica errores para lo demás.
     */
    void procesarLinea(const std::string& linea);

    bool salir_ = false;
};

#endif // CLI_H