// Punto de entrada del simulador: crea las estructuras compartidas, las agrupa
// en el ContextoCli y arranca la consola.
// Nota: todavia no hay hilo del motor; solo corre el CLI en el hilo principal.

#include "../include/Cli.h"
#include "../include/ContextoCli.h"
#include "../include/EstadoSimulador.h"
#include "../include/TablaProcesos.h"
#include "../include/ColaListos.h"
#include "../include/ListaLlegadas.h"
#include "../include/GestorProcesos.h"

int main() {
    EstadoSimulador estado;
    TablaProcesos tabla;
    ColaListos cola;
    ListaLlegadas llegadas;
    GestorProcesos gestor(estado, tabla, cola, llegadas);
    ContextoCli ctx{estado, tabla, cola, llegadas, gestor};

    Cli cli(ctx);
    cli.ejecutar();
    return 0;
}
