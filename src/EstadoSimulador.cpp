// EstadoSimulador.cpp - Implementacion del estado global del simulador.
// Dependencias: EstadoSimulador.h, Cerrojo.h
#include "EstadoSimulador.h"
#include "Cerrojo.h"

EstadoSimulador::EstadoSimulador()
    : clock_ticks_(0), ms_por_tick_(1000), politica_(Politica::RR),
      quantum_(2), pausado_(true), apagar_(false) {
    pthread_mutex_init(&mutex_, nullptr);
    pthread_cond_init(&cond_, nullptr);
}

EstadoSimulador::~EstadoSimulador() {
    pthread_cond_destroy(&cond_);
    pthread_mutex_destroy(&mutex_);
}

bool EstadoSimulador::set_politica(Politica p, int quantum) {
    if (quantum <= 0) return false;
    Cerrojo c(mutex_);
    politica_ = p;
    quantum_ = quantum;
    return true;
}

bool EstadoSimulador::set_velocidad(int ms_por_tick) {
    if (ms_por_tick < 0) return false;
    Cerrojo c(mutex_);
    ms_por_tick_ = ms_por_tick;
    return true;
}

void EstadoSimulador::iniciar() {
    Cerrojo c(mutex_);
    pausado_ = false;
    pthread_cond_broadcast(&cond_);
}

void EstadoSimulador::pausar() {
    Cerrojo c(mutex_);
    pausado_ = true;
}

void EstadoSimulador::pedir_apagado() {
    Cerrojo c(mutex_);
    apagar_ = true;
    // broadcast: el motor puede estar dormido en la condvar y debe salir aunque siga pausado.
    pthread_cond_broadcast(&cond_);
}

bool EstadoSimulador::esperar_si_pausado() {
    Cerrojo c(mutex_);
    // while y no if: pthread_cond_wait puede despertar espuriamente, hay que re-evaluar.
    while (pausado_ && !apagar_) pthread_cond_wait(&cond_, &mutex_);
    return !apagar_;
}

ConfigTick EstadoSimulador::leer_config() const {
    // Todo bajo un solo lock: con locks separados podria leerse (RR, quantum de SRTF).
    Cerrojo c(mutex_);
    return ConfigTick{politica_, quantum_, ms_por_tick_};
}

int EstadoSimulador::avanzar_reloj() {
    Cerrojo c(mutex_);
    return ++clock_ticks_;
}

int EstadoSimulador::leer_reloj() const {
    Cerrojo c(mutex_);
    return clock_ticks_;
}

bool EstadoSimulador::esta_pausado() const {
    Cerrojo c(mutex_);
    return pausado_;
}

bool EstadoSimulador::apagado() const {
    Cerrojo c(mutex_);
    return apagar_;
}
