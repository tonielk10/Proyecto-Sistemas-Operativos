#include "../include/ValidadorNombre.h"
#include <iostream>
#include <string>
#include <vector>

// Estructura para almacenar cada caso de prueba
struct CasoPrueba {
    std::string nombre;
    bool resultado_esperado;
};

int main() {
    // Cadenas auxiliares para probar longitudes exactas
    std::string nombre32(32, 'a'); // 32 letras 'a'
    std::string nombre33(33, 'a'); // 33 letras 'a'

    // Definición de los 14 casos de prueba exigidos por el Kanban
    std::vector<CasoPrueba> casos = {
        {"P1", true},
        {"Proceso_A", true},
        {"Job-1", true},
        {"a", true},
        {"Proceso_ñandú", true}, // Las letras con tilde y 'ñ' deben ser válidas
        {nombre32, true},        // Un nombre de exactamente 32 letras
        {nombre33, false},       // Un nombre de 33 letras
        {"", false},             // Vacío
        {"A B", false},          // Con espacio
        {"a\tb", false},         // Con tab
        {"a\nb", false},         // a seguido de salto de línea y b
        {"a,b", false},          // a,b
        {"\x01", false},         // un byte de control
        {"nom\x7f", false}       // nom seguido de \x7f (127)
    };

    int total_pasados = 0;

    for (size_t i = 0; i < casos.size(); ++i) {
        bool resultado_real = nombreValido(casos[i].nombre);

        if (resultado_real != casos[i].resultado_esperado) {
            std::cerr << "[FALLO] Caso " << i + 1 << " ('" << casos[i].nombre << "')\n"
                      << "  Esperaba: " << (casos[i].resultado_esperado ? "valido" : "invalido") << "\n"
                      << "  Obtuvo:   " << (resultado_real ? "valido" : "invalido") << "\n";
            return 1; // Termina con código 1 ante el primer fallo
        }
        total_pasados++;
    }

    std::cout << "Exito: Los " << total_pasados << " casos del Validador de Nombre pasaron correctamente.\n";
    return 0; // Termina con código 0 (éxito)
}