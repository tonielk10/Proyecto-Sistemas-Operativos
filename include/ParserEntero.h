// Códigos de estado que representan el resultado de la conversión de texto a número.

#ifndef PARSER_ENTERO_H
#define PARSER_ENTERO_H

#include <string>

/* Convierte un texto en un número entero de forma estricta, evadiendo colapsos del sistema.
en orden, el resultado puede ser: OK (El texto es un entero válido y está dentro del rango permitido.), NO_ENTERO
(El texto no representa un número entero válido.), FUERA_DE_RANGO (El número entero está fuera del rango permitido.).
 */
enum class ResultadoParseo { OK, NO_ENTERO, FUERA_DE_RANGO };

/*Convierte un texto en un número entero de forma estricta, evadiendo colapsos del sistema.
el parametro texto es la cadena a convertir.
el parametro minimo es el valor mínimo permitido.
el parametro maximo es el valor máximo permitido.
el parametro salida es la variable donde se almacenará el resultado. Su valor original no se modifica
a menos que la función retorne ResultadoParseo::OK.
en orden, el resultado puede ser: OK (El texto es un entero válido y está dentro del rango permitido.), NO_ENTERO
(El texto no representa un número entero válido.), FUERA_DE_RANGO (El número entero está fuera del rango permitido.).
*/
ResultadoParseo parsearEntero(const std::string& texto, long minimo, long maximo, long& salida);

#endif // PARSER_ENTERO_H