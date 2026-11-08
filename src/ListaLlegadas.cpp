// ListaLlegadas.cpp - Implementacion de la lista de llegadas.
// Dependencias: ListaLlegadas.h, Cerrojo.h
#include "ListaLlegadas.h"
#include "Cerrojo.h"

ListaLlegadas::ListaLlegadas() {
    pthread_mutex_init(&mutex_, nullptr);
}

ListaLlegadas::~ListaLlegadas() {
    pthread_mutex_destroy(&mutex_);
}

bool ListaLlegadas::agregar(int llegada, int pid) {
    if (llegada < 0 || pid <= 0) return false;
    Cerrojo c(mutex_);
    if (!pids_.insert(pid).second) return false;
    llegadas_.emplace(llegada, pid);
    return true;
}

std::vector<int> ListaLlegadas::extraer_hasta(int reloj) {
    Cerrojo c(mutex_);
    std::vector<int> salida;
    auto it = llegadas_.begin();
    while (it != llegadas_.end() && it->first <= reloj) {
        salida.push_back(it->second);
        pids_.erase(it->second);
        it = llegadas_.erase(it);
    }
    return salida;
}

bool ListaLlegadas::hay_pendientes() const {
    Cerrojo c(mutex_);
    return !llegadas_.empty();
}

std::size_t ListaLlegadas::tamano() const {
    Cerrojo c(mutex_);
    return llegadas_.size();
}
