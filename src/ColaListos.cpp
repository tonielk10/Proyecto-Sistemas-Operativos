// ColaListos.cpp - Implementacion de la cola de LISTOS.
// Dependencias: ColaListos.h, Cerrojo.h
#include "ColaListos.h"
#include "Cerrojo.h"

#include <algorithm>

ColaListos::ColaListos() {
    pthread_mutex_init(&mutex_, nullptr);
}

ColaListos::~ColaListos() {
    pthread_mutex_destroy(&mutex_);
}

bool ColaListos::encolar(int pid) {
    if (pid <= 0) return false;
    Cerrojo c(mutex_);
    if (std::find(cola_.begin(), cola_.end(), pid) != cola_.end()) return false;
    cola_.push_back(pid);
    return true;
}

bool ColaListos::desencolar(int& pid) {
    Cerrojo c(mutex_);
    if (cola_.empty()) return false;
    pid = cola_.front();
    cola_.pop_front();
    return true;
}

bool ColaListos::ver_primero(int& pid) const {
    Cerrojo c(mutex_);
    if (cola_.empty()) return false;
    pid = cola_.front();
    return true;
}

bool ColaListos::quitar(int pid) {
    Cerrojo c(mutex_);
    auto it = std::find(cola_.begin(), cola_.end(), pid);
    if (it == cola_.end()) return false;
    cola_.erase(it);
    return true;
}

bool ColaListos::contiene(int pid) const {
    Cerrojo c(mutex_);
    return std::find(cola_.begin(), cola_.end(), pid) != cola_.end();
}

std::vector<int> ColaListos::listar() const {
    Cerrojo c(mutex_);
    return std::vector<int>(cola_.begin(), cola_.end());
}

std::size_t ColaListos::tamano() const {
    Cerrojo c(mutex_);
    return cola_.size();
}

bool ColaListos::vacia() const {
    Cerrojo c(mutex_);
    return cola_.empty();
}
