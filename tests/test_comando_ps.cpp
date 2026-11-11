// Pruebas del comando ps: formato exacto, tabla vacia, parametros sobrantes y nombre largo.

#include "../include/Comandos.h"
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
    ContextoCli ctx{estado, tabla, cola, llegadas, gestor, false};
};

std::string capturarPs(Entorno& e, const std::vector<std::string>& args) {
    std::ostringstream buf;
    std::streambuf* viejo = std::cout.rdbuf(buf.rdbuf());
    comandoPs(args, e.ctx);
    std::cout.rdbuf(viejo);
    return buf.str();
}

void agregar(Entorno& e, int pid, const std::string& nombre, EstadoProceso est, int burst, int restante, int prio) {
    PCB p(pid, nombre, 0, burst, prio);
    p.estado = est;
    p.tiempo_restante = restante;
    e.tabla.agregar(p);
}

} // namespace

int main() {
    {
        Entorno e;
        agregar(e, 1, "Proceso_A", EstadoProceso::RUNNING, 5, 3, 2);
        agregar(e, 2, "Proceso_B", EstadoProceso::READY, 5, 5, 1);
        agregar(e, 3, "Proceso_C", EstadoProceso::READY, 2, 2, 3);
        agregar(e, 4, "Proceso_D", EstadoProceso::TERMINATED, 4, 0, 2);
        const std::string esperado =
            "PID    NOMBRE         ESTADO       BURST RESTANTE    PRIORIDAD\n"
            "------------------------------------------------------------\n"
            "1      Proceso_A      RUNNING      3s                2\n"
            "2      Proceso_B      READY        5s                1\n"
            "3      Proceso_C      READY        2s                3\n"
            "4      Proceso_D      TERMINATED   0s                2\n";
        CHECK(capturarPs(e, {}) == esperado);
        std::cout << "OK formato_cuatro_procesos\n";
    }
    {
        Entorno e;
        CHECK(capturarPs(e, {}) == "[INFO] No hay procesos en el sistema.\n");
        std::cout << "OK tabla_vacia\n";
    }
    {
        Entorno e;
        agregar(e, 1, "P", EstadoProceso::READY, 1, 1, 1);
        const std::string s = capturarPs(e, {"extra"});
        CHECK(s.rfind("[ERROR DE SINTAXIS]: Este comando no recibe parámetros.", 0) == 0);
        CHECK(s.find("Uso correcto: ps") != std::string::npos);
        CHECK(s.find("PID") == std::string::npos);
        std::cout << "OK args_sobrantes\n";
    }
    {
        Entorno e;
        const std::string largo(32, 'x');
        agregar(e, 1, largo, EstadoProceso::READY, 4, 4, 1);
        const std::string s = capturarPs(e, {});
        CHECK(s.find(largo + " READY") != std::string::npos);
        std::istringstream in(s);
        std::string linea;
        while (std::getline(in, linea)) {
            CHECK(!linea.empty() && linea.back() != ' ');
        }
        std::cout << "OK nombre_largo\n";
    }
    return 0;
}
