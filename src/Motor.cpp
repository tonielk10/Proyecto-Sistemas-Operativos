#include "../include/Motor.h"

#include <sched.h>
#include <unistd.h>

#include <algorithm>
#include <iostream>
#include <vector>

#include "../include/Mensajes.h"
#include "../include/PoliticaPlanificacion.h"

namespace {
VistaProceso vistaDe(const PCB& p) { return {p.PID, p.tiempo_restante, p.prioridad, p.tiempo_llegada}; }
}

Motor::Motor(EstadoSimulador& estado, TablaProcesos& tabla, ColaListos& cola,
             ListaLlegadas& llegadas, GestorProcesos& gestor, RegistroGantt& gantt)
    : estado_(estado), tabla_(tabla), cola_(cola), llegadas_(llegadas),
      gestor_(gestor), gantt_(gantt), hilo_(), corriendo_(false) {}

Motor::~Motor() { detener_y_unir(); }

void Motor::iniciar_hilo() {
    if (corriendo_) return;
    if (pthread_create(&hilo_, nullptr, &Motor::punto_entrada, this) == 0) corriendo_ = true;
}

void Motor::detener_y_unir() {
    if (!corriendo_) return;
    estado_.pedir_apagado();
    pthread_join(hilo_, nullptr);
    corriendo_ = false;
}

void* Motor::punto_entrada(void* arg) {
    static_cast<Motor*>(arg)->bucle();
    return nullptr;
}

void Motor::dormir_ms(int ms) {
    if (ms <= 0) {
        sched_yield();
        return;
    }
    int restante = ms;
    while (restante > 0 && !estado_.apagado()) {
        int tramo = std::min(restante, 20);
        usleep(static_cast<useconds_t>(tramo) * 1000);
        restante -= tramo;
    }
}

void Motor::bucle() {
    RoundRobin rr;
    Srtf srtf;
    int pid_actual = 0;
    int quantum_usado = 0;
    Politica anterior = Politica::RR;

    while (estado_.esperar_si_pausado()) {
        ConfigTick cfg = estado_.leer_config();
        int reloj = estado_.leer_reloj();
        if (cfg.politica != anterior) {
            quantum_usado = 0;
            anterior = cfg.politica;
        }

        // 1. llegadas: entran a la cola antes de reencolar al desalojado
        for (int pid : llegadas_.extraer_hasta(reloj)) gestor_.pasar_a_listo(pid);

        // 2. fotos
        PCB pcb(0, "", 0, 0, 0);
        VistaProceso vista_actual{};
        bool hay_actual = false;
        if (pid_actual != 0 && tabla_.obtener(pid_actual, pcb)) {
            vista_actual = vistaDe(pcb);
            hay_actual = true;
        }
        std::vector<VistaProceso> listos;
        for (int pid : cola_.listar()) {
            if (tabla_.obtener(pid, pcb)) listos.push_back(vistaDe(pcb));
        }

        // 3. nada que ejecutar
        if (!hay_actual && listos.empty()) {
            if (llegadas_.hay_pendientes()) {
                gantt_.registrar_tick(reloj, 0, "IDLE");
                estado_.avanzar_reloj();
                dormir_ms(cfg.ms_por_tick);
            } else {
                estado_.pausar();
                std::cout << Mensajes::mensajeInfo("Simulación finalizada: no quedan procesos pendientes.") << std::endl;
            }
            continue;
        }

        // 4. decidir
        const PoliticaPlanificacion& politica =
            (cfg.politica == Politica::SRTF) ? static_cast<const PoliticaPlanificacion&>(srtf) : rr;
        int elegido = politica.elegir(hay_actual ? &vista_actual : nullptr, listos, cfg.quantum, quantum_usado);

        // 5. cambio de contexto
        if (elegido != pid_actual) {
            if (pid_actual != 0) {
                gestor_.cambiar_estado(pid_actual, EstadoProceso::READY);
                cola_.encolar(pid_actual);
            }
            cola_.quitar(elegido);
            gestor_.cambiar_estado(elegido, EstadoProceso::RUNNING);
            tabla_.actualizar(elegido, [reloj](PCB& p) {
                if (p.tiempo_respuesta < 0) p.tiempo_respuesta = reloj - p.tiempo_llegada;
            });
            pid_actual = elegido;
            quantum_usado = 0;
        }

        // 6. ejecutar 1 tick
        bool termino = false;
        std::string nombre;
        tabla_.actualizar(pid_actual, [reloj, &termino, &nombre](PCB& p) {
            nombre = p.nombre;
            if (--p.tiempo_restante <= 0) {
                p.tiempo_restante = 0;
                p.cambiar_estado(EstadoProceso::TERMINATED);
                p.tiempo_retorno = (reloj + 1) - p.tiempo_llegada;
                termino = true;
            }
        });
        ++quantum_usado;
        for (int pid : cola_.listar()) tabla_.actualizar(pid, [](PCB& p) { ++p.tiempo_espera; });
        gantt_.registrar_tick(reloj, pid_actual, nombre);
        estado_.avanzar_reloj();
        if (termino) {
            pid_actual = 0;
            quantum_usado = 0;
        }
        dormir_ms(cfg.ms_por_tick);
    }
}
