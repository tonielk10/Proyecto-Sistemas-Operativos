// TablaProcesos.cpp - Implementacion de la tabla de procesos.
// Dependencias: TablaProcesos.h, Cerrojo.h
#include "TablaProcesos.h"
#include "Cerrojo.h"

TablaProcesos::TablaProcesos() {
    pthread_mutex_init(&mutex_, nullptr);
}

TablaProcesos::~TablaProcesos() {
    pthread_mutex_destroy(&mutex_);
}

bool TablaProcesos::agregar(const PCB& p) {
    if (p.PID <= 0) return false;
    Cerrojo c(mutex_);
    return procesos_.emplace(p.PID, p).second;
}

bool TablaProcesos::obtener(int pid, PCB& salida) const {
    Cerrojo c(mutex_);
    auto it = procesos_.find(pid);
    if (it == procesos_.end()) return false;
    salida = it->second;
    return true;
}

bool TablaProcesos::existe(int pid) const {
    Cerrojo c(mutex_);
    return procesos_.find(pid) != procesos_.end();
}

bool TablaProcesos::actualizar(int pid, const std::function<void(PCB&)>& fn) {
    Cerrojo c(mutex_);
    auto it = procesos_.find(pid);
    if (it == procesos_.end()) return false;
    // fn trabaja sobre una copia: si cambia el PID se descarta todo, porque la clave
    // del mapa dejaria de coincidir con el PCB guardado.
    PCB copia = it->second;
    fn(copia);
    if (copia.PID != pid) return false;
    it->second = copia;
    return true;
}

std::vector<PCB> TablaProcesos::listar() const {
    Cerrojo c(mutex_);
    std::vector<PCB> foto;
    foto.reserve(procesos_.size());
    for (const auto& par : procesos_) foto.push_back(par.second);
    return foto;
}

std::size_t TablaProcesos::tamano() const {
    Cerrojo c(mutex_);
    return procesos_.size();
}
