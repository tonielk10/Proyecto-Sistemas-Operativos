// Metricas.h - Resumen de metricas de planificacion (funcion pura).
// Dependencias: PCB.h, RegistroGantt.h
#ifndef METRICAS_H
#define METRICAS_H

#include <vector>

#include "PCB.h"
#include "RegistroGantt.h"

struct ResumenMetricas {
    int finalizados;
    int total;
    double espera_prom;
    double retorno_prom;
    double respuesta_prom;
    double uso_cpu;  // porcentaje 0..100
};

/** Promedios solo sobre los TERMINATED; uso_cpu = ticks no ociosos / ticks totales del Gantt * 100. */
ResumenMetricas calcularResumen(const std::vector<PCB>& procesos, const std::vector<SegmentoGantt>& gantt);

#endif
