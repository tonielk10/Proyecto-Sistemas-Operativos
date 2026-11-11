#include "../include/Tokenizador.h"
#include <iostream>
#include <string>
#include <vector>

// Estructura para almacenar cada caso de prueba
struct CasoPrueba {
    std::string entrada;
    std::vector<std::string> resultado_esperado;
};

// Función auxiliar para imprimir el vector en caso de fallo
void imprimirVector(const std::vector<std::string>& vec) {
    std::cout << "{ ";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << "\"" << vec[i] << "\"";
        if (i < vec.size() - 1) std::cout << ", ";
    }
    std::cout << " }";
}

int main() {
    // Definición de los 10 casos de prueba exigidos por el Kanban
    std::vector<CasoPrueba> casos = {
        {"create P1 5", {"create", "P1", "5"}},
        {" ps ", {"ps"}},
        {"create\tP1\t5", {"create", "P1", "5"}}, // Con tabs
        {"a     b", {"a", "b"}}, // Múltiples espacios
        {"", {}}, // Línea vacía
        {"    ", {}}, // Solo espacios
        {"\t\t", {}}, // Solo tabs
        {"pcb 1\r", {"pcb", "1"}}, // Con \r al final
        {"solo", {"solo"}},
        {"Create P1", {"Create", "P1"}} // Mayúsculas y minúsculas se mantienen
    };

    int total_pasados = 0;

    for (size_t i = 0; i < casos.size(); ++i) {
        std::vector<std::string> resultado_real = tokenizar(casos[i].entrada);

        // En C++, los vectores se pueden comparar directamente con ==
        if (resultado_real != casos[i].resultado_esperado) {
            std::cerr << "[FALLO] Caso " << i + 1 << " (Entrada: \"" << casos[i].entrada << "\")\n";
            
            std::cerr << "  Esperaba: ";
            imprimirVector(casos[i].resultado_esperado);
            std::cerr << "\n  Obtuvo:   ";
            imprimirVector(resultado_real);
            std::cerr << "\n";
            
            return 1; // Termina con código 1 ante el primer fallo
        }
        total_pasados++;
    }

    std::cout << "Exito: Los " << total_pasados << " casos del Tokenizador pasaron correctamente.\n";
    return 0; // Termina con código 0 (éxito)
}