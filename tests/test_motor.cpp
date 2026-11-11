// Pruebas de RoundRobin, Srtf, RegistroGantt, calcularResumen y del Motor (escenarios del enunciado).
// Compilar (normal):
//   FUENTES=$(grep -L "int main" src/*.cpp)
//   g++ -std=c++17 -Wall -Wextra -pthread -I. -Iinclude $FUENTES tests/test_motor.cpp -o /tmp/test_motor
//   timeout 60 /tmp/test_motor
// ThreadSanitizer: agregar -g -O1 -fsanitize=thread
#include "../include/ColaListos.h"
#include "../include/EstadoSimulador.h"
#include "../include/GestorProcesos.h"
#include "../include/ListaLlegadas.h"
#include "../include/Metricas.h"
#include "../include/Motor.h"
#include "../include/PCB.h"
#include "../include/PoliticaPlanificacion.h"
#include "../include/RegistroGantt.h"
#include "../include/TablaProcesos.h"

#include <unistd.h>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

#define CHECK(cond)                                                          \
    do {                                                                     \
        if (!(cond)) {                                                       \
            std::printf("FALLO linea %d: %s\n", __LINE__, #cond);            \
            return 1;                                                        \
        }                                                                    \
    } while (0)

struct Sistema {
    EstadoSimulador estado;
    TablaProcesos tabla;
    ColaListos cola;
    ListaLlegadas llegadas;
    GestorProcesos gestor{estado, tabla, cola, llegadas};
    RegistroGantt gantt;
    Motor motor{estado, tabla, cola, llegadas, gestor, gantt};
};

// Arranca el motor y espera (max 5 s) a que se pause solo al terminar. false si no termino.
static bool correr_hasta_el_final(Sistema& s) {
    s.motor.iniciar_hilo();
    s.estado.iniciar();
    for (int i = 0; i < 5000 && !s.estado.esta_pausado(); ++i) usleep(1000);
    bool ok = s.estado.esta_pausado();
    s.motor.detener_y_unir();
    return ok;
}

static PCB pcb_de(const Sistema& s, int pid) {
    PCB p(0, "", 0, 0, 0);
    s.tabla.obtener(pid, p);
    return p;
}

static bool metricas(const Sistema& s, int pid, int espera, int retorno, int respuesta) {
    PCB p = pcb_de(s, pid);
    return p.estado == EstadoProceso::TERMINATED && p.tiempo_espera == espera &&
           p.tiempo_retorno == retorno && p.tiempo_respuesta == respuesta;
}

static bool cerca(double a, double b) { return std::fabs(a - b) < 0.005; }

static VistaProceso v(int pid, int restante, int prioridad = 3, int llegada = 0) {
    return {pid, restante, prioridad, llegada};
}



int main() {
    {
        RoundRobin rr;
        VistaProceso a = v(1, 5);
        std::vector<VistaProceso> listos = {v(2, 3), v(3, 1)};
        CHECK(rr.elegir(&a, listos, 2, 1) == 1);   // quantum no agotado
        CHECK(rr.elegir(&a, listos, 2, 2) == 2);   // agotado: primero de la cola
        CHECK(rr.elegir(&a, {}, 2, 2) == 1);       // agotado sin listos: sigue el actual
        CHECK(rr.elegir(nullptr, listos, 2, 0) == 2);
        CHECK(rr.elegir(nullptr, {}, 2, 0) == 0);
        std::printf("OK RoundRobin::elegir\n");
    }
    {
        Srtf sr;
        VistaProceso a = v(5, 3, 3, 4);
        CHECK(sr.elegir(&a, {v(2, 4), v(3, 2)}, 2, 99) == 3);       // menor restante, ignora quantum
        CHECK(sr.elegir(&a, {v(2, 3, 3, 4)}, 2, 0) == 5);           // empate total: gana el actual
        CHECK(sr.elegir(&a, {v(2, 3, 1, 4)}, 2, 0) == 5);           // empate de restante: gana el actual aunque tenga mejor prioridad
        CHECK(sr.elegir(nullptr, {v(2, 3, 2), v(3, 3, 1)}, 2, 0) == 3);          // prioridad
        CHECK(sr.elegir(nullptr, {v(2, 3, 1, 5), v(3, 3, 1, 2)}, 2, 0) == 3);    // llegada
        CHECK(sr.elegir(nullptr, {v(7, 3, 1, 2), v(4, 3, 1, 2)}, 2, 0) == 4);    // pid
        CHECK(sr.elegir(nullptr, {}, 2, 0) == 0);
        std::printf("OK Srtf::elegir\n");
    }
    {
        RegistroGantt g;
        CHECK(formatearGantt(g.segmentos()) == "DIAGRAMA DE GANTT (0 - 0 ticks):\n[Sin datos]");
        g.registrar_tick(0, 1, "A");
        g.registrar_tick(1, 1, "A");
        g.registrar_tick(2, 2, "B");
        g.registrar_tick(5, 2, "B");  // hueco: abre otro segmento
        g.registrar_tick(6, 0, "IDLE");
        std::vector<SegmentoGantt> segs = g.segmentos();
        CHECK(segs.size() == 4);
        CHECK(segs[0].inicio == 0 && segs[0].fin == 2 && segs[1].inicio == 2 && segs[1].fin == 3);
        CHECK(formatearGantt(segs) ==
              "DIAGRAMA DE GANTT (0 - 7 ticks):\n[0-2s: A] -> [2-3s: B] -> [5-6s: B] -> [6-7s: IDLE]");
        std::printf("OK RegistroGantt y formatearGantt\n");
    }
    {
        ResumenMetricas vacio = calcularResumen({}, {});
        CHECK(vacio.finalizados == 0 && vacio.total == 0);
        CHECK(vacio.espera_prom == 0.0 && vacio.retorno_prom == 0.0 && vacio.respuesta_prom == 0.0);
        CHECK(vacio.uso_cpu == 0.0);

        PCB a(1, "A", 0, 3, 1), b(2, "B", 0, 3, 1), c(3, "C", 0, 3, 1);
        a.estado = EstadoProceso::TERMINATED;
        a.tiempo_espera = 2; a.tiempo_retorno = 5; a.tiempo_respuesta = 0;
        b.estado = EstadoProceso::TERMINATED;
        b.tiempo_espera = 4; b.tiempo_retorno = 8; b.tiempo_respuesta = 2;
        c.estado = EstadoProceso::RUNNING;  // no cuenta en los promedios
        c.tiempo_espera = 100; c.tiempo_retorno = -1; c.tiempo_respuesta = 9;
        std::vector<SegmentoGantt> gantt = {{0, 3, 1, "A"}, {3, 4, 0, "IDLE"}};
        ResumenMetricas r = calcularResumen({a, b, c}, gantt);
        CHECK(r.finalizados == 2 && r.total == 3);
        CHECK(cerca(r.espera_prom, 3.0) && cerca(r.retorno_prom, 6.5) && cerca(r.respuesta_prom, 1.0));
        CHECK(cerca(r.uso_cpu, 75.0));

        ResumenMetricas sin = calcularResumen({c}, gantt);
        CHECK(sin.finalizados == 0 && sin.total == 1 && sin.espera_prom == 0.0 && sin.retorno_prom == 0.0);
        std::printf("OK calcularResumen\n");
    }
    {
        Sistema s;
        s.estado.set_velocidad(0);
        s.estado.set_politica(Politica::RR, 2);
        int p1 = s.gestor.crear_proceso("P1", 5, 2);
        int p2 = s.gestor.crear_proceso("P2", 3, 1);
        int p3 = s.gestor.crear_proceso("P3", 2, 3);
        CHECK(correr_hasta_el_final(s));
        std::vector<SegmentoGantt> g = s.gantt.segmentos();
        CHECK(formatearGantt(g) ==
              "DIAGRAMA DE GANTT (0 - 10 ticks):\n"
              "[0-2s: P1] -> [2-4s: P2] -> [4-6s: P3] -> [6-8s: P1] -> [8-9s: P2] -> [9-10s: P1]");
        CHECK(metricas(s, p1, 5, 10, 0));
        CHECK(metricas(s, p2, 6, 9, 2));
        CHECK(metricas(s, p3, 4, 6, 4));
        ResumenMetricas r = calcularResumen(s.tabla.listar(), g);
        CHECK(r.finalizados == 3 && r.total == 3);
        CHECK(cerca(r.espera_prom, 5.0) && cerca(r.retorno_prom, 8.33));
        CHECK(cerca(r.uso_cpu, 100.0));
        std::printf("OK escenario 1 (Round Robin, quantum 2)\n");
    }
    {
        Sistema s;
        s.estado.set_velocidad(0);
        s.estado.set_politica(Politica::SRTF, 2);
        int a = s.gestor.registrar_llegada_futura("Job_A", 4, 2, 0);
        int b = s.gestor.registrar_llegada_futura("Job_B", 2, 1, 1);
        int c = s.gestor.registrar_llegada_futura("Job_C", 1, 3, 3);
        CHECK(correr_hasta_el_final(s));
        CHECK(formatearGantt(s.gantt.segmentos()) ==
              "DIAGRAMA DE GANTT (0 - 7 ticks):\n"
              "[0-1s: Job_A] -> [1-3s: Job_B] -> [3-4s: Job_C] -> [4-7s: Job_A]");
        CHECK(metricas(s, a, 3, 7, 0));
        CHECK(metricas(s, b, 0, 2, 0));
        CHECK(metricas(s, c, 0, 1, 0));
        std::printf("OK escenario 2 (SRTF con llegadas)\n");
    }
    {
        Sistema s;
        s.estado.set_velocidad(0);
        s.estado.set_politica(Politica::RR, 2);
        s.gestor.registrar_llegada_futura("X", 2, 3, 3);
        CHECK(correr_hasta_el_final(s));
        std::vector<SegmentoGantt> g = s.gantt.segmentos();
        CHECK(formatearGantt(g) == "DIAGRAMA DE GANTT (0 - 5 ticks):\n[0-3s: IDLE] -> [3-5s: X]");
        CHECK(cerca(calcularResumen(s.tabla.listar(), g).uso_cpu, 40.0));
        std::printf("OK IDLE inicial\n");
    }
    {
        Sistema s;
        s.estado.set_velocidad(1);
        s.estado.set_politica(Politica::RR, 2);
        int suma = 0;
        for (int i = 0; i < 20; ++i) {
            int burst = 1 + (i % 4);
            suma += burst;
            CHECK(s.gestor.crear_proceso("c" + std::to_string(i), burst, 1 + (i % 5)) > 0);
        }
        s.motor.iniciar_hilo();
        s.estado.iniciar();
        for (int i = 0; i < 200; ++i) {
            s.estado.set_politica(i % 2 == 0 ? Politica::RR : Politica::SRTF, 2);
            s.estado.set_velocidad(1 + (i % 3));
            usleep(300);
        }
        for (int i = 0; i < 5000 && !s.estado.esta_pausado(); ++i) usleep(1000);
        CHECK(s.estado.esta_pausado());
        s.motor.detener_y_unir();

        std::vector<PCB> todos = s.tabla.listar();
        CHECK(todos.size() == 20);
        for (const PCB& p : todos) CHECK(p.estado == EstadoProceso::TERMINATED && p.tiempo_restante == 0);
        int ocupados = 0;
        for (const SegmentoGantt& seg : s.gantt.segmentos()) {
            if (seg.pid != 0) ocupados += seg.fin - seg.inicio;
        }
        CHECK(ocupados == suma);
        std::printf("OK concurrencia (cambios de politica y velocidad con el motor corriendo)\n");
    }
    return 0;
}
