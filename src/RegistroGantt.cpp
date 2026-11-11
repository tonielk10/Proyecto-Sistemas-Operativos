#include "../include/RegistroGantt.h"

#include "../include/Cerrojo.h"

RegistroGantt::RegistroGantt() { pthread_mutex_init(&mutex_, nullptr); }

RegistroGantt::~RegistroGantt() { pthread_mutex_destroy(&mutex_); }

void RegistroGantt::registrar_tick(int tick, int pid, const std::string& nombre) {
    Cerrojo c(mutex_);
    if (!segmentos_.empty()) {
        SegmentoGantt& ultimo = segmentos_.back();
        if (ultimo.pid == pid && ultimo.fin == tick) {
            ultimo.fin = tick + 1;
            return;
        }
    }
    segmentos_.push_back({tick, tick + 1, pid, nombre});
}

std::vector<SegmentoGantt> RegistroGantt::segmentos() const {
    Cerrojo c(mutex_);
    return segmentos_;
}

std::string formatearGantt(const std::vector<SegmentoGantt>& segs) {
    int fin = segs.empty() ? 0 : segs.back().fin;
    std::string texto = "DIAGRAMA DE GANTT (0 - " + std::to_string(fin) + " ticks):\n";
    if (segs.empty()) return texto + "[Sin datos]";
    for (std::size_t i = 0; i < segs.size(); ++i) {
        if (i > 0) texto += " -> ";
        texto += "[" + std::to_string(segs[i].inicio) + "-" + std::to_string(segs[i].fin) +
                 "s: " + segs[i].nombre + "]";
    }
    return texto;
}
