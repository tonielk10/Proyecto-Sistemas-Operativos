// Pruebas de EstadoSimulador.
// Compilar (normal):
//   g++ -std=c++17 -Wall -Wextra -pthread -Iinclude src/EstadoSimulador.cpp tests/test_estado.cpp -o /tmp/test_estado
//   timeout 10 /tmp/test_estado
// Compilar (ThreadSanitizer):
//   g++ -std=c++17 -Wall -Wextra -pthread -g -O1 -fsanitize=thread -Iinclude src/EstadoSimulador.cpp tests/test_estado.cpp -o /tmp/test_estado_tsan
//   timeout 30 /tmp/test_estado_tsan
#include "../include/EstadoSimulador.h"

#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <cstdio>

#define CHECK(cond)                                                          \
    do {                                                                     \
        if (!(cond)) {                                                       \
            std::printf("FALLO linea %d: %s\n", __LINE__, #cond);            \
            return 1;                                                        \
        }                                                                    \
    } while (0)

static const int N = 20000;

static void* hilo_a(void* arg) {
    EstadoSimulador* e = static_cast<EstadoSimulador*>(arg);
    for (int i = 0; i < N; ++i) {
        e->set_politica(Politica::RR, 2);
        e->set_velocidad(i % 100);
        e->set_politica(Politica::SRTF, 7);
    }
    return nullptr;
}

static void* hilo_b(void* arg) {
    EstadoSimulador* e = static_cast<EstadoSimulador*>(arg);
    long mezclados = 0;
    for (int i = 0; i < N; ++i) {
        ConfigTick c = e->leer_config();
        bool ok = (c.politica == Politica::RR && c.quantum == 2) ||
                  (c.politica == Politica::SRTF && c.quantum == 7);
        if (!ok) ++mezclados;
    }
    return reinterpret_cast<void*>(mezclados);
}

static void* hilo_c(void* arg) {
    EstadoSimulador* e = static_cast<EstadoSimulador*>(arg);
    for (int i = 0; i < N; ++i) e->avanzar_reloj();
    return nullptr;
}

static void* hilo_espera(void* arg) {
    EstadoSimulador* e = static_cast<EstadoSimulador*>(arg);
    return e->esperar_si_pausado() ? reinterpret_cast<void*>(1) : nullptr;
}

int main() {
    {
        EstadoSimulador e;
        ConfigTick c = e.leer_config();
        CHECK(e.leer_reloj() == 0);
        CHECK(c.ms_por_tick == 1000);
        CHECK(c.politica == Politica::RR);
        CHECK(c.quantum == 2);
        CHECK(e.esta_pausado());
        CHECK(!e.apagado());
        std::printf("OK valores por defecto\n");
    }
    {
        EstadoSimulador e;
        CHECK(!e.set_politica(Politica::RR, 0));
        CHECK(!e.set_politica(Politica::RR, -1));
        CHECK(!e.set_velocidad(-5));
        ConfigTick c = e.leer_config();
        CHECK(c.politica == Politica::RR && c.quantum == 2 && c.ms_por_tick == 1000);
        std::printf("OK setters invalidos\n");
    }
    {
        EstadoSimulador e;
        pthread_t a, b, c;
        pthread_create(&a, nullptr, hilo_a, &e);
        pthread_create(&b, nullptr, hilo_b, &e);
        pthread_create(&c, nullptr, hilo_c, &e);
        void* mezclados = nullptr;
        pthread_join(a, nullptr);
        pthread_join(b, &mezclados);
        pthread_join(c, nullptr);
        CHECK(mezclados == nullptr);
        CHECK(e.leer_reloj() == N);
        std::printf("OK estres de concurrencia\n");
    }
    {
        EstadoSimulador e;
        pthread_t t;
        pthread_create(&t, nullptr, hilo_espera, &e);
        timespec antes, despues;
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &antes);
        usleep(1500000);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &despues);
        double cpu = static_cast<double>(despues.tv_sec - antes.tv_sec) +
                     static_cast<double>(despues.tv_nsec - antes.tv_nsec) / 1e9;
        CHECK(cpu < 0.1);
        e.iniciar();
        void* r = nullptr;
        pthread_join(t, &r);
        CHECK(r != nullptr);
        std::printf("OK sin busy-waiting (cpu=%.3f s)\n", cpu);
    }
    {
        EstadoSimulador e;
        pthread_t t;
        pthread_create(&t, nullptr, hilo_espera, &e);
        usleep(200000);
        e.pedir_apagado();
        void* r = reinterpret_cast<void*>(1);
        pthread_join(t, &r);
        CHECK(r == nullptr);
        CHECK(e.apagado());
        std::printf("OK apagado despierta al motor\n");
    }
    {
        EstadoSimulador e;
        e.iniciar();
        e.iniciar();
        CHECK(!e.esta_pausado());
        e.pausar();
        CHECK(e.esta_pausado());
        std::printf("OK idempotencia de iniciar\n");
    }
    return 0;
}
