// Punto de entrada del simulador: crea las estructuras compartidas, las agrupa
// en el ContextoCli y arranca la consola.
// El motor corre en su propio hilo; el CLI corre en el hilo principal.

#include "../include/Cli.h"
#include "../include/ContextoCli.h"
#include "../include/EstadoSimulador.h"
#include "../include/TablaProcesos.h"
#include "../include/ColaListos.h"
#include "../include/ListaLlegadas.h"
#include "../include/GestorProcesos.h"
#include "../include/Motor.h"
#include "../include/RegistroGantt.h"

int main() {
    EstadoSimulador estado;
    TablaProcesos tabla;
    ColaListos cola;
    ListaLlegadas llegadas;
    GestorProcesos gestor(estado, tabla, cola, llegadas);
    RegistroGantt gantt;
    Motor motor(estado, tabla, cola, llegadas, gestor, gantt);
    ContextoCli ctx{estado, tabla, cola, llegadas, gestor};
    ctx.gantt = &gantt;

    motor.iniciar_hilo();
    Cli cli(ctx);
    cli.ejecutar();
    motor.detener_y_unir();
    return 0;
}
