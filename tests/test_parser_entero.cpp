#include "../include/ParserEntero.h"
#include <iostream>
#include <string>
#include <vector>

// Función auxiliar para imprimir el enum en texto y facilitar la lectura de errores
std::string resultadoAString(ResultadoParseo res) {
    switch (res) {
        case ResultadoParseo::OK: return "OK";
        case ResultadoParseo::NO_ENTERO: return "NO_ENTERO";
        case ResultadoParseo::FUERA_DE_RANGO: return "FUERA_DE_RANGO";
        default: return "DESCONOCIDO";
    }
}

// Estructura para almacenar cada caso de prueba
struct CasoPrueba {
    std::string texto;
    long minimo;
    long maximo;
    ResultadoParseo resultado_esperado;
    long valor_esperado; // Solo se evalúa si el resultado_esperado es OK
};

int main() {
    // Definición de los 22 casos de prueba exigidos
    std::vector<CasoPrueba> casos = {
        {"5", 1, 100, ResultadoParseo::OK, 5},
        {"1", 1, 100, ResultadoParseo::OK, 1},
        {"100", 1, 100, ResultadoParseo::OK, 100},
        {"007", 1, 100, ResultadoParseo::OK, 7},
        {"0", 1, 100, ResultadoParseo::FUERA_DE_RANGO, 0},
        {"101", 1, 100, ResultadoParseo::FUERA_DE_RANGO, 0},
        {"-3", 1, 100, ResultadoParseo::FUERA_DE_RANGO, 0},
        {"99999999999999999999", 1, 100, ResultadoParseo::FUERA_DE_RANGO, 0},
        {"-3", -10, 100, ResultadoParseo::OK, -3}, // Caso especial con minimo = -10
        {"", 1, 100, ResultadoParseo::NO_ENTERO, 0},
        {"abc", 1, 100, ResultadoParseo::NO_ENTERO, 0},
        {"5x", 1, 100, ResultadoParseo::NO_ENTERO, 0},
        {"x5", 1, 100, ResultadoParseo::NO_ENTERO, 0},
        {"1e3", 1, 100, ResultadoParseo::NO_ENTERO, 0},
        {"3.5", 1, 100, ResultadoParseo::NO_ENTERO, 0},
        {"+5", 1, 100, ResultadoParseo::NO_ENTERO, 0},
        {"+", 1, 100, ResultadoParseo::NO_ENTERO, 0},
        {"-", 1, 100, ResultadoParseo::NO_ENTERO, 0},
        {"--5", 1, 100, ResultadoParseo::NO_ENTERO, 0},
        {"5 5", 1, 100, ResultadoParseo::NO_ENTERO, 0},
        {" 5", 1, 100, ResultadoParseo::NO_ENTERO, 0},
        {"5  ", 1, 100, ResultadoParseo::NO_ENTERO, 0}
    };

    int total_pasados = 0;

    for (size_t i = 0; i < casos.size(); ++i) {
        const auto& caso = casos[i];
        
        long salida = -999; // Valor inicial basura para comprobar que no se modifica en errores
        
        ResultadoParseo resultado_real = parsearEntero(caso.texto, caso.minimo, caso.maximo, salida);

        // Verificar si el resultado del enum es el esperado
        bool paso = (resultado_real == caso.resultado_esperado);

        // Si se esperaba OK, también hay que verificar que el número devuelto sea correcto
        if (paso && caso.resultado_esperado == ResultadoParseo::OK) {
            if (salida != caso.valor_esperado) {
                paso = false;
                std::cerr << "[FALLO] Caso " << i + 1 << " ('" << caso.texto << "'):\n"
                          << "  Esperaba valor de salida: " << caso.valor_esperado << "\n"
                          << "  Pero obtuvo: " << salida << "\n";
            }
        } 
        // Si no se esperaba OK, validar estrictamente que 'salida' no fue modificada
        else if (paso && caso.resultado_esperado != ResultadoParseo::OK) {
            if (salida != -999) {
                paso = false;
                std::cerr << "[FALLO] Caso " << i + 1 << " ('" << caso.texto << "'):\n"
                          << "  El resultado fue correcto (" << resultadoAString(resultado_real) 
                          << "), pero la variable 'salida' fue modificada indebidamente a: " << salida << "\n";
            }
        }

        if (!paso) {
            std::cerr << "[FALLO] Caso " << i + 1 << " ('" << caso.texto << "'):\n"
                      << "  Esperaba: " << resultadoAString(caso.resultado_esperado) << "\n"
                      << "  Obtuvo: " << resultadoAString(resultado_real) << "\n";
            return 1; // Termina con código 1 ante el primer fallo, como exige la issue
        }

        total_pasados++;
    }

    std::cout << "Exito: Los " << total_pasados << " casos pasaron correctamente.\n";
    return 0; // Termina con código 0 si todo está perfecto
}