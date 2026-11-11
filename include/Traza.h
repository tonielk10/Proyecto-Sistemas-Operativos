// Traza.h - Parser puro de archivos de trazas de trabajo (sin archivos ni hilos).
// Formato por linea: NOMBRE, BURST_TIME, PRIORIDAD, ARRIVAL_TIME
#ifndef TRAZA_H
#define TRAZA_H

#include <cstddef>
#include <istream>
#include <string>
#include <vector>

struct EntradaTraza {
    std::string nombre;
    int burst;
    int prioridad;
    int llegada;
};

struct ErrorTraza {
    int linea;            // numero de linea desde 1
    std::string detalle;
};

struct ResultadoTraza {
    std::vector<EntradaTraza> entradas;
    std::vector<ErrorTraza> errores;
};

constexpr std::size_t TRAZA_MAX_PROCESOS = 10000;
constexpr std::size_t TRAZA_MAX_LINEA = 1024;

/** Lee toda la entrada. Registra a lo sumo un error por linea; las lineas validas van a entradas. */
ResultadoTraza parsearTraza(std::istream& entrada);

#endif
