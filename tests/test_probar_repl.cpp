#include "../include/Cli.h"
#include "../include/EstadoSimulador.h"
#include "../include/TablaProcesos.h"
#include "../include/ColaListos.h"
#include "../include/ListaLlegadas.h"
#include "../include/GestorProcesos.h"
#include "../include/ContextoCli.h"

int main() {
    // 1. Crear los objetos bases del sistema
    EstadoSimulador estado;
    TablaProcesos tabla;
    ColaListos cola;
    ListaLlegadas llegadas;
    
    // Inicializar el gestor inyectándole las referencias que necesita
    GestorProcesos gestor{estado, tabla, cola, llegadas};

    // 2. Empaquetarlos en el contexto
    ContextoCli ctx{estado, tabla, cola, llegadas, gestor, false};

    // 3. Pasar el contexto al CLI y arrancar
    Cli cli(ctx);
    cli.ejecutar();

    return 0;
}