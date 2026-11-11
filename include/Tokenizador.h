#ifndef TOKENIZADOR_H
#define TOKENIZADOR_H

#include <string>
#include <vector>

/* Separa una línea de texto en palabras (tokens) utilizando espacios,
tabulaciones y saltos de línea como delimitadores.
el parametro linea es la cadena de texto capturada de la consola.
devuelve Un vector de strings con cada palabra separada. Retorna un vector 
vacío si la línea está vacía o contiene únicamente separadores.
 */
std::vector<std::string> tokenizar(const std::string& linea);

#endif // TOKENIZADOR_H