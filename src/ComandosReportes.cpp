// Comandos de reporte: pcb, gantt y stats.

#include "../include/Comandos.h"
#include "../include/Mensajes.h"
#include "../include/Metricas.h"
#include "../include/ParserEntero.h"
#include <cstdio>
#include <iostream>
#include <string>

namespace {

// Rellena con espacios hasta "ancho" caracteres contando puntos de codigo UTF-8, no bytes
std::string etiqueta(const std::string& texto, std::size_t ancho) {
    std::size_t largo = 0;
    for (unsigned char c : texto) {
        if ((c & 0xC0) != 0x80) ++largo;
    }
    return texto + std::string(largo < ancho ? ancho - largo : 0, ' ') + ": ";
}

std::string ticks(int n) { return std::to_string(n) + " ticks"; }

std::string decimales(double v, const char* formato) {
    char buf[32];
    std::snprintf(buf, sizeof buf, formato, v);
    return buf;
}

} // namespace

void comandoPcb(const std::vector<std::string>& args, ContextoCli& ctx) {
    if (args.empty()) {
        std::cout << Mensajes::errorSintaxis("Parámetros insuficientes.", Mensajes::USO_PCB) << "\n";
        return;
    }
    if (args.size() > 1) {
        std::cout << Mensajes::errorSintaxis("Demasiados parámetros.", Mensajes::USO_PCB) << "\n";
        return;
    }
    long pid = 0;
    if (parsearEntero(args[0], 1, 2147483647, pid) != ResultadoParseo::OK) {
        std::cout << Mensajes::errorTipo("El PID debe ser un número entero positivo.") << "\n";
        return;
    }
    PCB p(0, "", 0, 0, 0);
    if (!ctx.tabla.obtener(static_cast<int>(pid), p)) {
        std::cout << Mensajes::errorGeneral("PID " + std::to_string(pid) + " no existe") << "\n";
        return;
    }
    const std::string borde(50, '=');
    std::cout << borde << "\n"
              << "        CONTROL BLOCK DE PROCESO (PCB) - PID: " << p.PID << "\n"
              << borde << "\n"
              << etiqueta("Nombre del Proceso", 19) << p.nombre << "\n"
              << etiqueta("Estado Actual", 19) << nombreEstado(p.estado) << "\n"
              << etiqueta("Prioridad", 19) << p.prioridad << "\n"
              << etiqueta("Tiempo de Llegada", 19) << ticks(p.tiempo_llegada) << "\n"
              << etiqueta("Ráfaga Inicial", 19) << ticks(p.tiempo_rafaga) << "\n"
              << etiqueta("Ráfaga Restante", 19) << ticks(p.tiempo_restante) << "\n"
              << etiqueta("Tiempo de Espera", 19) << ticks(p.tiempo_espera) << "\n"
              << etiqueta("Tiempo de Retorno", 19)
              << (p.tiempo_retorno == -1 ? std::string("N/A (En ejecución)") : ticks(p.tiempo_retorno)) << "\n"
              << etiqueta("Tiempo de Respuesta", 19)
              << (p.tiempo_respuesta == -1 ? std::string("N/A") : ticks(p.tiempo_respuesta)) << "\n"
              << borde << "\n";
}

void comandoGantt(const std::vector<std::string>& args, ContextoCli& ctx) {
    if (!args.empty()) {
        std::cout << Mensajes::errorSintaxis("Este comando no recibe parámetros.", Mensajes::USO_GANTT) << "\n";
        return;
    }
    if (ctx.gantt == nullptr) {
        std::cout << Mensajes::errorGeneral("El diagrama de Gantt no está disponible.") << "\n";
        return;
    }
    std::cout << formatearGantt(ctx.gantt->segmentos()) << "\n";
}

void comandoStats(const std::vector<std::string>& args, ContextoCli& ctx) {
    if (!args.empty()) {
        std::cout << Mensajes::errorSintaxis("Este comando no recibe parámetros.", Mensajes::USO_STATS) << "\n";
        return;
    }
    std::vector<SegmentoGantt> segmentos;
    if (ctx.gantt != nullptr) segmentos = ctx.gantt->segmentos();
    ResumenMetricas r = calcularResumen(ctx.tabla.listar(), segmentos);
    std::cout << "ESTADÍSTICAS DEL SISTEMA:\n"
              << etiqueta("- Procesos Finalizados", 30) << r.finalizados << " / " << r.total << "\n"
              << etiqueta("- Tiempo Promedio de Espera", 30) << decimales(r.espera_prom, "%.2f") << " ticks\n"
              << etiqueta("- Tiempo Promedio de Retorno", 30) << decimales(r.retorno_prom, "%.2f") << " ticks\n"
              << etiqueta("- Tiempo Promedio de Respuesta", 30) << decimales(r.respuesta_prom, "%.2f") << " ticks\n"
              << etiqueta("- Uso de CPU", 30) << decimales(r.uso_cpu, "%.1f") << "%\n";
}
