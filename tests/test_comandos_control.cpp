// Pruebas de start, pause, help, set_sched, set_speed, pcb, gantt y stats.
// Compilar: FUENTES=$(grep -L "int main" src/*.cpp)
//   g++ -std=c++17 -Wall -Wextra -pthread -I. -Iinclude $FUENTES tests/test_comandos_control.cpp -o /tmp/test_comandos_control

#include "../include/Comandos.h"
#include "../include/Mensajes.h"
#include "../include/RegistroGantt.h"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#define CHECK(cond) do { if (!(cond)) { \
    std::cerr << "FALLO linea " << __LINE__ << ": " #cond "\n"; return 1; } } while (0)

namespace {

struct Entorno {
    EstadoSimulador estado;
    TablaProcesos tabla;
    ColaListos cola;
    ListaLlegadas llegadas;
    GestorProcesos gestor{estado, tabla, cola, llegadas};
    RegistroGantt gantt;
    ContextoCli ctx{estado, tabla, cola, llegadas, gestor, false};
    Entorno() { ctx.gantt = &gantt; }
};

std::string capturar(ManejadorComando f, Entorno& e, const std::vector<std::string>& args) {
    std::ostringstream buf;
    std::streambuf* viejo = std::cout.rdbuf(buf.rdbuf());
    f(args, e.ctx);
    std::cout.rdbuf(viejo);
    return buf.str();
}

void agregar(Entorno& e, int pid, const std::string& nombre, EstadoProceso est, int llegada, int burst,
             int restante, int prio, int espera, int retorno, int respuesta) {
    PCB p(pid, nombre, llegada, burst, prio);
    p.estado = est;
    p.tiempo_restante = restante;
    p.tiempo_espera = espera;
    p.tiempo_retorno = retorno;
    p.tiempo_respuesta = respuesta;
    e.tabla.agregar(p);
}

const std::string BORDE(50, '=');

} // namespace

int main() {
    {
        Entorno e;
        agregar(e, 1, "Proceso_A", EstadoProceso::RUNNING, 0, 5, 3, 2, 2, -1, 0);
        const std::string esperado = BORDE + "\n"
            "        CONTROL BLOCK DE PROCESO (PCB) - PID: 1\n" + BORDE + "\n"
            "Nombre del Proceso : Proceso_A\n"
            "Estado Actual      : RUNNING\n"
            "Prioridad          : 2\n"
            "Tiempo de Llegada  : 0 ticks\n"
            "Ráfaga Inicial     : 5 ticks\n"
            "Ráfaga Restante    : 3 ticks\n"
            "Tiempo de Espera   : 2 ticks\n"
            "Tiempo de Retorno  : N/A (En ejecución)\n"
            "Tiempo de Respuesta: 0 ticks\n" + BORDE + "\n";
        CHECK(capturar(comandoPcb, e, {"1"}) == esperado);

        agregar(e, 2, "B", EstadoProceso::TERMINATED, 1, 2, 0, 1, 0, 2, 0);
        std::string fin = capturar(comandoPcb, e, {"2"});
        CHECK(fin.find("Tiempo de Retorno  : 2 ticks\n") != std::string::npos);
        CHECK(fin.find("Estado Actual      : TERMINATED\n") != std::string::npos);

        agregar(e, 3, "C", EstadoProceso::READY, 0, 2, 2, 3, 0, -1, -1);
        CHECK(capturar(comandoPcb, e, {"3"}).find("Tiempo de Respuesta: N/A\n") != std::string::npos);

        const std::string uso = "\nUso correcto: " + Mensajes::USO_PCB + "\n";
        CHECK(capturar(comandoPcb, e, {}) == "[ERROR DE SINTAXIS]: Parámetros insuficientes." + uso);
        CHECK(capturar(comandoPcb, e, {"1", "2"}) == "[ERROR DE SINTAXIS]: Demasiados parámetros." + uso);
        const std::string tipo = "[ERROR DE TIPO]: El PID debe ser un número entero positivo.\n";
        CHECK(capturar(comandoPcb, e, {"abc"}) == tipo);
        CHECK(capturar(comandoPcb, e, {"-1"}) == tipo);
        CHECK(capturar(comandoPcb, e, {"0"}) == tipo);
        CHECK(capturar(comandoPcb, e, {"99999999999999999999"}) == tipo);
        CHECK(capturar(comandoPcb, e, {"99"}) == "[ERROR]: PID 99 no existe\n");
        std::cout << "OK pcb\n";
    }
    {
        Entorno e;
        CHECK(capturar(comandoSetSched, e, {"RR"}) == "[OK] Política: RR (quantum=2)\n");
        ConfigTick c = e.estado.leer_config();
        CHECK(c.politica == Politica::RR && c.quantum == 2);
        CHECK(capturar(comandoSetSched, e, {"RR", "5"}) == "[OK] Política: RR (quantum=5)\n");
        c = e.estado.leer_config();
        CHECK(c.politica == Politica::RR && c.quantum == 5);
        CHECK(capturar(comandoSetSched, e, {"SRTF"}) == "[OK] Política: SRTF\n");
        c = e.estado.leer_config();
        CHECK(c.politica == Politica::SRTF && c.quantum == 5);
        e.estado.set_politica(Politica::RR, 4);
        CHECK(capturar(comandoSetSched, e, {"SRTF", "3"}) ==
              "[INFO] El quantum no aplica a SRTF y se ignora.\n[OK] Política: SRTF\n");
        c = e.estado.leer_config();
        CHECK(c.politica == Politica::SRTF && c.quantum == 4);

        e.estado.set_politica(Politica::RR, 7);
        const std::vector<std::vector<std::string>> invalidos = {
            {"FIFO"}, {"rr"}, {"RR", "0"}, {"RR", "x"}, {"RR", "5", "9"}, {}, {"SRTF", "0"}, {"RR", "1001"}};
        for (const auto& args : invalidos) {
            std::string salida = capturar(comandoSetSched, e, args);
            CHECK(salida.rfind("[ERROR", 0) == 0);
            c = e.estado.leer_config();
            CHECK(c.politica == Politica::RR && c.quantum == 7);
        }
        CHECK(capturar(comandoSetSched, e, {"RR", "x"}) == "[ERROR DE TIPO]: El quantum debe ser un número entero.\n");
        CHECK(capturar(comandoSetSched, e, {"RR", "0"}) == "[ERROR DE PARAMETRO]: El quantum debe estar entre 1 y 1000.\n");
        std::cout << "OK set_sched\n";
    }
    {
        Entorno e;
        CHECK(capturar(comandoSetSpeed, e, {"500"}) == "[OK] Velocidad: 500 ms/tick\n");
        CHECK(e.estado.leer_config().ms_por_tick == 500);
        for (const std::vector<std::string>& args : std::vector<std::vector<std::string>>{
                 {"49"}, {"5001"}, {"abc"}, {"99999999999999999999"}, {}, {"100", "200"}}) {
            CHECK(capturar(comandoSetSpeed, e, args).rfind("[ERROR", 0) == 0);
            CHECK(e.estado.leer_config().ms_por_tick == 500);
        }
        CHECK(capturar(comandoSetSpeed, e, {"abc"}) ==
              "[ERROR DE TIPO]: La velocidad debe ser un número entero en milisegundos.\n");
        CHECK(capturar(comandoSetSpeed, e, {"49"}) ==
              "[ERROR DE PARAMETRO]: La velocidad debe estar entre 50 y 5000 ms.\n");
        std::cout << "OK set_speed\n";
    }
    {
        Entorno e;
        CHECK(e.estado.esta_pausado());
        CHECK(capturar(comandoStart, e, {}) == "[OK] Simulación iniciada.\n");
        CHECK(!e.estado.esta_pausado());
        CHECK(capturar(comandoStart, e, {}) == "[INFO] La simulación ya está en ejecución.\n");
        CHECK(capturar(comandoPause, e, {}) == "[OK] Simulación en pausa (termina el tick actual).\n");
        CHECK(e.estado.esta_pausado());
        CHECK(capturar(comandoPause, e, {}) == "[INFO] La simulación ya está en pausa.\n");
        CHECK(capturar(comandoStart, e, {"x"}).rfind("[ERROR DE SINTAXIS]", 0) == 0);
        CHECK(e.estado.esta_pausado());
        e.estado.iniciar();
        CHECK(capturar(comandoPause, e, {"x"}).rfind("[ERROR DE SINTAXIS]", 0) == 0);
        CHECK(!e.estado.esta_pausado());
        std::cout << "OK start_pause\n";
    }
    {
        Entorno e;
        CHECK(capturar(comandoGantt, e, {}) == "DIAGRAMA DE GANTT (0 - 0 ticks):\n[Sin datos]\n");
        e.gantt.registrar_tick(0, 1, "Job_A");
        e.gantt.registrar_tick(1, 2, "Job_B");
        e.gantt.registrar_tick(2, 2, "Job_B");
        CHECK(capturar(comandoGantt, e, {}) ==
              "DIAGRAMA DE GANTT (0 - 3 ticks):\n[0-1s: Job_A] -> [1-3s: Job_B]\n");
        CHECK(capturar(comandoGantt, e, {"x"}).rfind("[ERROR DE SINTAXIS]", 0) == 0);
        e.ctx.gantt = nullptr;
        CHECK(capturar(comandoGantt, e, {}) == "[ERROR]: El diagrama de Gantt no está disponible.\n");
        std::cout << "OK gantt\n";
    }
    {
        Entorno e;
        const std::string sin =
            "ESTADÍSTICAS DEL SISTEMA:\n"
            "- Procesos Finalizados        : 0 / 0\n"
            "- Tiempo Promedio de Espera   : 0.00 ticks\n"
            "- Tiempo Promedio de Retorno  : 0.00 ticks\n"
            "- Tiempo Promedio de Respuesta: 0.00 ticks\n"
            "- Uso de CPU                  : 0.0%\n";
        CHECK(capturar(comandoStats, e, {}) == sin);

        agregar(e, 1, "A", EstadoProceso::TERMINATED, 0, 3, 0, 1, 2, 5, 0);
        agregar(e, 2, "B", EstadoProceso::TERMINATED, 0, 3, 0, 1, 3, 7, 1);
        agregar(e, 3, "C", EstadoProceso::READY, 0, 3, 3, 1, 0, -1, -1);
        agregar(e, 4, "D", EstadoProceso::RUNNING, 0, 3, 2, 1, 0, -1, 0);
        for (int t = 0; t < 8; ++t) e.gantt.registrar_tick(t, 1, "A");
        CHECK(capturar(comandoStats, e, {}) ==
              "ESTADÍSTICAS DEL SISTEMA:\n"
              "- Procesos Finalizados        : 2 / 4\n"
              "- Tiempo Promedio de Espera   : 2.50 ticks\n"
              "- Tiempo Promedio de Retorno  : 6.00 ticks\n"
              "- Tiempo Promedio de Respuesta: 0.50 ticks\n"
              "- Uso de CPU                  : 100.0%\n");
        CHECK(capturar(comandoStats, e, {"x"}).rfind("[ERROR DE SINTAXIS]", 0) == 0);
        std::cout << "OK stats\n";
    }
    {
        Entorno e;
        std::string h = capturar(comandoHelp, e, {});
        CHECK(!h.empty());
        for (const std::string& uso : {Mensajes::USO_CREATE, Mensajes::USO_PS, Mensajes::USO_PCB,
                                       Mensajes::USO_SET_SCHED, Mensajes::USO_SET_SPEED, Mensajes::USO_START,
                                       Mensajes::USO_PAUSE, Mensajes::USO_GANTT, Mensajes::USO_STATS,
                                       Mensajes::USO_EXIT})
            CHECK(h.find(uso) != std::string::npos);
        CHECK(!e.ctx.salir);
        CHECK(capturar(comandoHelp, e, {"x"}).rfind("[ERROR DE SINTAXIS]", 0) == 0);
        std::cout << "OK help\n";
    }
    return 0;
}
