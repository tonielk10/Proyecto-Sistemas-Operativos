#include "../include/Metricas.h"

ResumenMetricas calcularResumen(const std::vector<PCB>& procesos, const std::vector<SegmentoGantt>& gantt) {
    ResumenMetricas r{0, static_cast<int>(procesos.size()), 0.0, 0.0, 0.0, 0.0};
    double espera = 0, retorno = 0, respuesta = 0;
    for (const PCB& p : procesos) {
        if (p.estado != EstadoProceso::TERMINATED) continue;
        ++r.finalizados;
        espera += p.tiempo_espera;
        retorno += p.tiempo_retorno;
        respuesta += p.tiempo_respuesta;
    }
    if (r.finalizados > 0) {
        r.espera_prom = espera / r.finalizados;
        r.retorno_prom = retorno / r.finalizados;
        r.respuesta_prom = respuesta / r.finalizados;
    }
    int ocupados = 0;
    for (const SegmentoGantt& s : gantt) {
        if (s.pid != 0) ocupados += s.fin - s.inicio;
    }
    int total = gantt.empty() ? 0 : gantt.back().fin;
    if (total > 0) r.uso_cpu = static_cast<double>(ocupados) / total * 100.0;
    return r;
}
