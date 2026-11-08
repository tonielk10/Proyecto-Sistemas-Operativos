// GestorProcesos.cpp - Implementacion del gestor de procesos.
// Dependencias: GestorProcesos.h, Cerrojo.h
#include "GestorProcesos.h"
#include "Cerrojo.h"

#include <cctype>

GestorProcesos::GestorProcesos(EstadoSimulador& estado, TablaProcesos& tabla,
                               ColaListos& cola, ListaLlegadas& llegadas)
    : ultimo_pid_(0), estado_(estado), tabla_(tabla), cola_(cola), llegadas_(llegadas) {
    pthread_mutex_init(&mutex_, nullptr);
}

GestorProcesos::~GestorProcesos() {
    pthread_mutex_destroy(&mutex_);
}

bool GestorProcesos::datos_validos(const std::string& nombre, int burst, int prioridad) {
    if (nombre.empty() || burst <= 0 || prioridad < 1 || prioridad > 5) return false;
    for (char ch : nombre) {
        unsigned char u = static_cast<unsigned char>(ch);
        if (std::isspace(u) || std::iscntrl(u)) return false;
    }
    return true;
}

int GestorProcesos::siguiente_pid() {
    Cerrojo c(mutex_);
    return ++ultimo_pid_;
}

int GestorProcesos::crear_proceso(const std::string& nombre, int burst, int prioridad) {
    // Validar antes de pedir PID: un dato invalido no debe dejar huecos en la numeracion.
    if (!datos_validos(nombre, burst, prioridad)) return -1;
    int pid = siguiente_pid();
    if (!tabla_.agregar(PCB(pid, nombre, estado_.leer_reloj(), burst, prioridad))) return -1;
    if (!pasar_a_listo(pid)) return -1;
    return pid;
}

int GestorProcesos::registrar_llegada_futura(const std::string& nombre, int burst,
                                             int prioridad, int llegada) {
    if (llegada < 0 || !datos_validos(nombre, burst, prioridad)) return -1;
    int pid = siguiente_pid();
    if (!tabla_.agregar(PCB(pid, nombre, llegada, burst, prioridad))) return -1;
    if (!llegadas_.agregar(llegada, pid)) return -1;
    return pid;
}

bool GestorProcesos::pasar_a_listo(int pid) {
    // Primero READY en la tabla y recien despues encolar: en cuanto el PID entra a la
    // cola el motor puede sacarlo y pasarlo a RUNNING, y NEW -> RUNNING es invalido.
    bool era_nuevo = false;
    bool existe = tabla_.actualizar(pid, [&era_nuevo](PCB& p) {
        if (p.estado != EstadoProceso::NEW) return;
        era_nuevo = p.cambiar_estado(EstadoProceso::READY);
    });
    if (!existe || !era_nuevo) return false;

    if (!cola_.encolar(pid)) {
        // No deberia pasar (un proceso en NEW no esta en la cola), pero si pasa se
        // deshace el cambio para no dejar un READY fuera de la cola. No es una transicion
        // del ciclo de vida sino una vuelta atras, por eso no pasa por cambiar_estado.
        tabla_.actualizar(pid, [](PCB& p) {
            if (p.estado == EstadoProceso::READY) p.estado = EstadoProceso::NEW;
        });
        return false;
    }
    return true;
}

bool GestorProcesos::cambiar_estado(int pid, EstadoProceso nuevo) {
    bool cambiado = false;
    bool existe = tabla_.actualizar(pid, [&cambiado, nuevo](PCB& p) {
        cambiado = p.cambiar_estado(nuevo);
    });
    return existe && cambiado;
}

int GestorProcesos::ultimo_pid() const {
    Cerrojo c(mutex_);
    return ultimo_pid_;
}
