#include "../include/PoliticaPlanificacion.h"

#include <tuple>

int RoundRobin::elegir(const VistaProceso* actual, const std::vector<VistaProceso>& listos,
                       int quantum, int quantum_usado) const {
    if (actual != nullptr && quantum_usado < quantum) return actual->pid;
    if (!listos.empty()) return listos.front().pid;
    return actual != nullptr ? actual->pid : 0;
}

int Srtf::elegir(const VistaProceso* actual, const std::vector<VistaProceso>& listos,
                 int /*quantum*/, int /*quantum_usado*/) const {
    // clave: restante, actual primero, prioridad (menor = mas alta), llegada, pid
    using Clave = std::tuple<int, int, int, int, int>;
    int mejor_pid = 0;
    Clave mejor{};
    bool hay = false;
    auto probar = [&](const VistaProceso& v, int no_actual) {
        Clave k{v.restante, no_actual, v.prioridad, v.llegada, v.pid};
        if (!hay || k < mejor) {
            mejor = k;
            mejor_pid = v.pid;
            hay = true;
        }
    };
    if (actual != nullptr) probar(*actual, 0);
    for (const VistaProceso& v : listos) probar(v, 1);
    return mejor_pid;
}
