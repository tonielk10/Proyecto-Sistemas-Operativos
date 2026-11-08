// Pruebas de GestorProcesos y de las transiciones de estado del PCB.
// Compilar (normal):
//   FUENTES="src/EstadoSimulador.cpp src/TablaProcesos.cpp src/ColaListos.cpp src/ListaLlegadas.cpp src/GestorProcesos.cpp"
//   g++ -std=c++17 -Wall -Wextra -pthread -I. -Iinclude $FUENTES tests/test_gestor.cpp -o /tmp/test_gestor
//   timeout 30 /tmp/test_gestor
// Compilar (ThreadSanitizer):
//   g++ -std=c++17 -Wall -Wextra -pthread -g -O1 -fsanitize=thread -I. -Iinclude $FUENTES tests/test_gestor.cpp -o /tmp/test_gestor_tsan
//   timeout 90 /tmp/test_gestor_tsan
// Valgrind:
//   g++ -std=c++17 -pthread -g -I. -Iinclude $FUENTES tests/test_gestor.cpp -o /tmp/test_gestor_g
//   valgrind --leak-check=full --error-exitcode=1 /tmp/test_gestor_g
//   valgrind --tool=helgrind /tmp/test_gestor_g
//
// Las pruebas de transiciones invalidas imprimen "[ERROR INTERNO] ..." en stderr a proposito.
#include "ColaListos.h"
#include "EstadoSimulador.h"
#include "GestorProcesos.h"
#include "ListaLlegadas.h"
#include "PCB.h"
#include "TablaProcesos.h"

#include <pthread.h>
#include <sched.h>
#include <cstdio>
#include <vector>

#define CHECK(cond)                                                          \
    do {                                                                     \
        if (!(cond)) {                                                       \
            std::printf("FALLO linea %d: %s\n", __LINE__, #cond);            \
            return 1;                                                        \
        }                                                                    \
    } while (0)

static const int HILOS = 4;
static const int POR_HILO = 2500;
static const int TOTAL = HILOS * POR_HILO;

// Agrupa las estructuras que el gestor usa; cada prueba arma uno nuevo.
struct Sistema {
    EstadoSimulador estado;
    TablaProcesos tabla;
    ColaListos cola;
    ListaLlegadas llegadas;
    GestorProcesos gestor{estado, tabla, cola, llegadas};
};

static EstadoProceso estado_de(const TablaProcesos& t, int pid) {
    PCB p(0, "", 0, 0, 0);
    t.obtener(pid, p);
    return p.estado;
}

// --- Concurrencia ---

struct Creador {
    GestorProcesos* gestor;
    std::vector<int> pids;
};

static void* hilo_creador(void* arg) {
    Creador* c = static_cast<Creador*>(arg);
    for (int i = 0; i < POR_HILO; ++i) c->pids.push_back(c->gestor->crear_proceso("proc", 4));
    return nullptr;
}

struct Consumidor {
    ColaListos* cola;
    GestorProcesos* gestor;
    std::vector<int> pids;
    long fallos_transicion;
};

static void* hilo_consumidor(void* arg) {
    Consumidor* c = static_cast<Consumidor*>(arg);
    while (static_cast<int>(c->pids.size()) < TOTAL) {
        int pid = 0;
        if (!c->cola->desencolar(pid)) {
            sched_yield();
            continue;
        }
        if (!c->gestor->cambiar_estado(pid, EstadoProceso::RUNNING)) ++c->fallos_transicion;
        c->pids.push_back(pid);
    }
    return nullptr;
}

// Cada PID de 1..TOTAL debe aparecer exactamente una vez.
static bool permutacion_completa(const std::vector<int>& pids) {
    if (static_cast<int>(pids.size()) != TOTAL) return false;
    std::vector<int> vistos(TOTAL + 1, 0);
    for (int pid : pids) {
        if (pid < 1 || pid > TOTAL || vistos[pid]++ != 0) return false;
    }
    return true;
}

int main() {
    {
        using E = EstadoProceso;
        PCB p(1, "p", 0, 3, 1);
        CHECK(p.estado == E::NEW);

        CHECK(transicionValida(E::NEW, E::READY));
        CHECK(transicionValida(E::READY, E::RUNNING));
        CHECK(transicionValida(E::RUNNING, E::READY));
        CHECK(transicionValida(E::RUNNING, E::WAITING));
        CHECK(transicionValida(E::RUNNING, E::TERMINATED));
        CHECK(transicionValida(E::WAITING, E::READY));
        CHECK(!transicionValida(E::TERMINATED, E::RUNNING));
        CHECK(!transicionValida(E::NEW, E::RUNNING));
        CHECK(!transicionValida(E::READY, E::TERMINATED));
        CHECK(!transicionValida(E::RUNNING, E::RUNNING));

        CHECK(!p.cambiar_estado(E::RUNNING));
        CHECK(p.estado == E::NEW);
        CHECK(p.cambiar_estado(E::READY) && p.estado == E::READY);
        std::printf("OK PCB y transiciones\n");
    }
    {
        Sistema s;
        CHECK(s.gestor.ultimo_pid() == 0);
        CHECK(s.gestor.crear_proceso("a", 5) == 1);
        CHECK(s.gestor.crear_proceso("b", 2, 1) == 2);
        CHECK(s.gestor.ultimo_pid() == 2);

        PCB p(0, "", 0, 0, 0);
        CHECK(s.tabla.obtener(1, p));
        CHECK(p.estado == EstadoProceso::READY && p.prioridad == 3 && p.tiempo_llegada == 0);
        CHECK(p.tiempo_rafaga == 5 && p.tiempo_restante == 5 && p.nombre == "a");
        CHECK(s.cola.listar() == (std::vector<int>{1, 2}));

        s.estado.avanzar_reloj();
        s.estado.avanzar_reloj();
        s.estado.avanzar_reloj();
        CHECK(s.gestor.crear_proceso("c", 1) == 3);
        CHECK(s.tabla.obtener(3, p) && p.tiempo_llegada == 3);
        std::printf("OK crear_proceso\n");
    }
    {
        Sistema s;
        CHECK(s.gestor.crear_proceso("a", 0) == -1);
        CHECK(s.gestor.crear_proceso("a", -4) == -1);
        CHECK(s.gestor.crear_proceso("a", 3, 0) == -1);
        CHECK(s.gestor.crear_proceso("a", 3, 6) == -1);
        CHECK(s.gestor.crear_proceso("", 3) == -1);
        CHECK(s.gestor.crear_proceso("dos palabras", 3) == -1);
        CHECK(s.gestor.crear_proceso("tab\t", 3) == -1);
        CHECK(s.gestor.registrar_llegada_futura("a", 0, 3, 5) == -1);
        CHECK(s.tabla.tamano() == 0 && s.gestor.ultimo_pid() == 0);

        CHECK(s.gestor.crear_proceso("ok", 3) == 1);
        CHECK(s.gestor.crear_proceso("ok", 3, 5) == 2);  // nombre repetido permitido
        CHECK(s.tabla.tamano() == 2);
        std::printf("OK datos invalidos sin huecos de PID\n");
    }
    {
        Sistema s;
        CHECK(s.gestor.registrar_llegada_futura("f", 4, 2, -1) == -1);
        int pid = s.gestor.registrar_llegada_futura("f", 4, 2, 7);
        CHECK(pid == 1);
        CHECK(estado_de(s.tabla, pid) == EstadoProceso::NEW);
        CHECK(s.llegadas.tamano() == 1 && s.cola.vacia());
        PCB p(0, "", 0, 0, 0);
        CHECK(s.tabla.obtener(pid, p) && p.tiempo_llegada == 7);

        // Simula lo que hara el motor al llegar el tick 7.
        CHECK(s.llegadas.extraer_hasta(7) == (std::vector<int>{pid}));
        CHECK(s.gestor.pasar_a_listo(pid));
        CHECK(estado_de(s.tabla, pid) == EstadoProceso::READY);
        CHECK(s.cola.listar() == (std::vector<int>{pid}));
        CHECK(!s.gestor.pasar_a_listo(pid));
        CHECK(!s.gestor.pasar_a_listo(99));
        CHECK(s.cola.tamano() == 1);
        std::printf("OK registrar_llegada_futura y pasar_a_listo\n");
    }
    {
        Sistema s;
        int pid = s.gestor.crear_proceso("x", 2);
        CHECK(s.gestor.cambiar_estado(pid, EstadoProceso::RUNNING));
        CHECK(s.gestor.cambiar_estado(pid, EstadoProceso::TERMINATED));
        CHECK(!s.gestor.cambiar_estado(pid, EstadoProceso::RUNNING));
        CHECK(estado_de(s.tabla, pid) == EstadoProceso::TERMINATED);
        CHECK(!s.gestor.cambiar_estado(42, EstadoProceso::READY));
        std::printf("OK cambiar_estado del gestor\n");
    }
    {
        Sistema s;
        Creador creadores[HILOS];
        pthread_t hilos[HILOS];
        Consumidor consumidor{&s.cola, &s.gestor, {}, 0};
        pthread_t hc;
        pthread_create(&hc, nullptr, hilo_consumidor, &consumidor);
        for (int i = 0; i < HILOS; ++i) {
            creadores[i].gestor = &s.gestor;
            pthread_create(&hilos[i], nullptr, hilo_creador, &creadores[i]);
        }
        for (int i = 0; i < HILOS; ++i) pthread_join(hilos[i], nullptr);
        pthread_join(hc, nullptr);

        std::vector<int> creados;
        for (const Creador& c : creadores) creados.insert(creados.end(), c.pids.begin(), c.pids.end());
        CHECK(permutacion_completa(creados));
        CHECK(permutacion_completa(consumidor.pids));
        CHECK(consumidor.fallos_transicion == 0);
        CHECK(s.tabla.tamano() == static_cast<size_t>(TOTAL));
        CHECK(s.gestor.ultimo_pid() == TOTAL);
        CHECK(s.cola.vacia());
        std::printf("OK concurrencia (%d procesos, 4 creadores + 1 consumidor)\n", TOTAL);
    }
    return 0;
}
