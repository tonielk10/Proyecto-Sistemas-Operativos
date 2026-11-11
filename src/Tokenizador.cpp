#include "../include/Tokenizador.h"

std::vector<std::string> tokenizar(const std::string& linea) {
    std::vector<std::string> tokens;
    
    //los separadores son el espacio, el tab (\t), \r y \n.
    const std::string delimitadores = " \t\r\n"; 
    
    // Buscar el primer carácter que NO sea un separador
    size_t inicio = linea.find_first_not_of(delimitadores);
    
    while (inicio != std::string::npos) {
        // Buscar el siguiente separador a partir del inicio actual
        size_t fin = linea.find_first_of(delimitadores, inicio);
        
        // Extraer la palabra y guardarla en el vector
        tokens.push_back(linea.substr(inicio, fin - inicio));
        
        // Buscar el inicio de la siguiente palabra (ignorando separadores múltiples)
        inicio = linea.find_first_not_of(delimitadores, fin);
    }
    
    //Nunca devuelve palabras vacías. Si solo había espacios, el vector queda vacío.
    return tokens;
}