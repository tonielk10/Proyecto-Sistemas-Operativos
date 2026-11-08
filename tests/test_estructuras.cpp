// Pruebas de TablaProcesos, ColaListos y ListaLlegadas.
// Compilar (normal):
//   g++ -std=c++17 -Wall -Wextra -pthread -I. -Iinclude src/TablaProcesos.cpp src/ColaListos.cpp src/ListaLlegadas.cpp tests/test_estructuras.cpp -o /tmp/test_estructuras
//   timeout 20 /tmp/test_estructuras
// Compilar (ThreadSanitizer):
//   g++ -std=c++17 -Wall -Wextra -pthread -g -O1 -fsanitize=thread -I. -Iinclude src/TablaProcesos.cpp src/ColaListos.cpp src/ListaLlegadas.cpp tests/test_estructuras.cpp -o /tmp/test_estructuras_tsan
//   timeout 60 /tmp/test_estructuras_tsan
// Valgrind:
//   g++ -std=c++17 -pthread -g -I. -Iinclude src/TablaProcesos.cpp src/ColaListos.cpp src/ListaLlegadas.cpp tests/test_estructuras.cpp -o /tmp/test_estructuras_g
//   valgrind --leak-check=full --error-exitcode=1 /tmp/test_estructuras_g
//   valgrind --tool=helgrind /tmp/test_estructuras_g
#include "ListaLlegadas.h"
#include "TablaProcesos.h"
#include "ColaListos.h"

#include <pthread.h>
#include <sched.h>
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

static const int N = 10000;

// El constructor de PCB no inicializa estado; se fija aqui para no copiar basura.
static PCB nuevo_pcb(int pid, int llegada = 0, int rafaga = 5) {
    PCB p(pid, "p" + std::to_string(pid), llegada, rafaga, 1);
    p.estado = EstadoProceso::NEW;
    return p;
}

// --- Estres TablaProcesos ---

static void* tp_escritor(void* arg) {
    TablaProcesos* t = static_cast<TablaProcesos*>(arg);
    for (int pid = 1; pid <= N; ++pid) t->agregar(nuevo_pcb(pid));
    return nullptr;
}

struct LectorTP {
    TablaProcesos* tabla;
    long errores;
};

static void* tp_lector(void* arg) {
    LectorTP* l = static_cast<LectorTP*>(arg);
    PCB p = nuevo_pcb(1);
    for (int i = 0; i < N; ++i) {
        if (i % 100 == 0) {
            std::vector<PCB> foto = l->tabla->listar();
            for (size_t k = 1; k < foto.size(); ++k)
                if (foto[k - 1].PID >= foto[k].PID) ++l->errores;
        }
        int pid = (i % N) + 1;
        if (l->tabla->obtener(pid, p) && p.PID != pid) ++l->errores;
        if (l->tabla->tamano() > static_cast<size_t>(N)) ++l->errores;
    }
    return nullptr;
}

// --- Estres ColaListos ---

static void* cl_productor(void* arg) {
    ColaListos* q = static_cast<ColaListos*>(arg);
    for (int pid = 1; pid <= N; ++pid) q->encolar(pid);
    return nullptr;
}

struct ConsumidorCL {
    ColaListos* cola;
    long fuera_de_orden;
};

static void* cl_consumidor(void* arg) {
    ConsumidorCL* c = static_cast<ConsumidorCL*>(arg);
    int esperado = 1;
    while (esperado <= N) {
        int pid = 0;
        if (!c->cola->desencolar(pid)) {
            sched_yield();
            continue;
        }
        if (pid != esperado) ++c->fuera_de_orden;
        ++esperado;
    }
    return nullptr;
}

// --- Estres ListaLlegadas ---
// Llegada de cada pid = pid % 100, asi hay muchos empates resueltos por PID.

static void* ll_productor(void* arg) {
    ListaLlegadas* a = static_cast<ListaLlegadas*>(arg);
    for (int pid = 1; pid <= N; ++pid) a->agregar(pid % 100, pid);
    return nullptr;
}

struct ExtractorLL {
    ListaLlegadas* lista;
    std::vector<int> vistos;  // veces que salio cada pid
    long total;
    long desordenados;
};

static void* ll_extractor(void* arg) {
    ExtractorLL* e = static_cast<ExtractorLL*>(arg);
    while (e->total < N) {
        std::vector<int> lote = e->lista->extraer_hasta(100);
        if (lote.empty()) {
            sched_yield();
            continue;
        }
        for (size_t k = 0; k < lote.size(); ++k) {
            ++e->vistos[lote[k]];
            ++e->total;
            if (k > 0) {
                int a = lote[k - 1], b = lote[k];
                bool ordenado = (a % 100 < b % 100) || (a % 100 == b % 100 && a < b);
                if (!ordenado) ++e->desordenados;
            }
        }
    }
    return nullptr;
}

int main() {
    {
        TablaProcesos t;
        CHECK(t.agregar(nuevo_pcb(5)));
        CHECK(t.agregar(nuevo_pcb(2)));
        CHECK(t.agregar(nuevo_pcb(9)));
        CHECK(!t.agregar(nuevo_pcb(5)));
        CHECK(!t.agregar(nuevo_pcb(0)));
        CHECK(!t.agregar(nuevo_pcb(-3)));
        CHECK(t.tamano() == 3);
        CHECK(t.existe(2) && !t.existe(7));

        PCB copia = nuevo_pcb(1);
        CHECK(t.obtener(5, copia));
        CHECK(copia.PID == 5 && copia.nombre == "p5");
        copia.nombre = "modificado";
        copia.tiempo_restante = 99;
        PCB otra = nuevo_pcb(1);
        CHECK(t.obtener(5, otra));
        CHECK(otra.nombre == "p5" && otra.tiempo_restante == 5);

        CHECK(t.actualizar(5, [](PCB& p) { p.tiempo_restante = 1; p.estado = EstadoProceso::READY; }));
        CHECK(t.obtener(5, otra));
        CHECK(otra.tiempo_restante == 1 && otra.estado == EstadoProceso::READY);
        CHECK(!t.actualizar(7, [](PCB& p) { p.tiempo_restante = 0; }));
        CHECK(!t.actualizar(5, [](PCB& p) { p.PID = 42; p.tiempo_restante = 0; }));
        CHECK(t.obtener(5, otra) && otra.tiempo_restante == 1 && !t.existe(42));

        PCB intacto = nuevo_pcb(1);
        CHECK(!t.obtener(7, intacto));
        CHECK(intacto.PID == 1);

        std::vector<PCB> foto = t.listar();
        CHECK(foto.size() == 3);
        CHECK(foto[0].PID == 2 && foto[1].PID == 5 && foto[2].PID == 9);
        std::printf("OK TablaProcesos funcional\n");
    }
    {
        ColaListos q;
        int pid = 0;
        CHECK(q.vacia());
        CHECK(!q.desencolar(pid));
        CHECK(!q.ver_primero(pid));
        CHECK(q.encolar(3) && q.encolar(1) && q.encolar(4) && q.encolar(2));
        CHECK(!q.encolar(1));
        CHECK(!q.encolar(0));
        CHECK(q.tamano() == 4 && !q.vacia());
        CHECK(q.contiene(4) && !q.contiene(8));

        CHECK(q.quitar(4));
        CHECK(!q.quitar(4));
        CHECK(q.listar() == (std::vector<int>{3, 1, 2}));

        CHECK(q.ver_primero(pid) && pid == 3);
        CHECK(q.tamano() == 3);
        CHECK(q.desencolar(pid) && pid == 3);
        CHECK(q.desencolar(pid) && pid == 1);
        CHECK(q.desencolar(pid) && pid == 2);
        CHECK(!q.desencolar(pid));
        CHECK(q.vacia());
        std::printf("OK ColaListos funcional\n");
    }
    {
        ListaLlegadas a;
        CHECK(!a.hay_pendientes());
        CHECK(a.agregar(5, 3));
        CHECK(a.agregar(2, 7));
        CHECK(a.agregar(2, 4));
        CHECK(a.agregar(0, 9));
        CHECK(a.agregar(8, 1));
        CHECK(!a.agregar(1, 3));
        CHECK(!a.agregar(-1, 10));
        CHECK(!a.agregar(1, 0));
        CHECK(a.tamano() == 5);

        CHECK(a.extraer_hasta(-1).empty());
        CHECK(a.extraer_hasta(2) == (std::vector<int>{9, 4, 7}));
        CHECK(a.tamano() == 2 && a.hay_pendientes());
        CHECK(a.extraer_hasta(4).empty());
        CHECK(a.extraer_hasta(5) == (std::vector<int>{3}));
        CHECK(a.agregar(1, 4));  // el pid ya salio, se puede volver a agregar
        CHECK(a.extraer_hasta(100) == (std::vector<int>{4, 1}));
        CHECK(!a.hay_pendientes() && a.tamano() == 0);
        std::printf("OK ListaLlegadas funcional\n");
    }
    {
        TablaProcesos t;
        LectorTP lector{&t, 0};
        pthread_t ha, hb;
        pthread_create(&ha, nullptr, tp_escritor, &t);
        pthread_create(&hb, nullptr, tp_lector, &lector);
        pthread_join(ha, nullptr);
        pthread_join(hb, nullptr);
        CHECK(lector.errores == 0);
        CHECK(t.tamano() == static_cast<size_t>(N));
        std::printf("OK estres TablaProcesos\n");
    }
    {
        ColaListos q;
        ConsumidorCL consumidor{&q, 0};
        pthread_t hp, hc;
        pthread_create(&hp, nullptr, cl_productor, &q);
        pthread_create(&hc, nullptr, cl_consumidor, &consumidor);
        pthread_join(hp, nullptr);
        pthread_join(hc, nullptr);
        CHECK(consumidor.fuera_de_orden == 0);
        CHECK(q.vacia());
        std::printf("OK estres ColaListos\n");
    }
    {
        ListaLlegadas a;
        ExtractorLL extractor{&a, std::vector<int>(N + 1, 0), 0, 0};
        pthread_t hp, he;
        pthread_create(&hp, nullptr, ll_productor, &a);
        pthread_create(&he, nullptr, ll_extractor, &extractor);
        pthread_join(hp, nullptr);
        pthread_join(he, nullptr);
        CHECK(extractor.total == N);
        CHECK(extractor.desordenados == 0);
        for (int pid = 1; pid <= N; ++pid) CHECK(extractor.vistos[pid] == 1);
        CHECK(!a.hay_pendientes());
        std::printf("OK estres ListaLlegadas\n");
    }
    return 0;
}
