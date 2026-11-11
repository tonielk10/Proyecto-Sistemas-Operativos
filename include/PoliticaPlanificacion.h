// PoliticaPlanificacion.h - Politicas de planificacion como funciones puras sobre una vista de los procesos.
// Dependencias: <vector>
#ifndef POLITICA_PLANIFICACION_H
#define POLITICA_PLANIFICACION_H

#include <vector>

struct VistaProceso {
    int pid;
    int restante;
    int prioridad;
    int llegada;
};

class PoliticaPlanificacion {
public:
    virtual ~PoliticaPlanificacion() = default;
    /**
     * actual: proceso RUNNING (nullptr si no hay). listos: cola READY en orden.
     * Devuelve el PID que corre en este tick (0 si no hay ninguno).
     */
    virtual int elegir(const VistaProceso* actual, const std::vector<VistaProceso>& listos,
                       int quantum, int quantum_usado) const = 0;
};

class RoundRobin : public PoliticaPlanificacion {
public:
    int elegir(const VistaProceso* actual, const std::vector<VistaProceso>& listos,
               int quantum, int quantum_usado) const override;
};

// Expropiativa por menor tiempo restante; ignora el quantum.
class Srtf : public PoliticaPlanificacion {
public:
    int elegir(const VistaProceso* actual, const std::vector<VistaProceso>& listos,
               int quantum, int quantum_usado) const override;
};

#endif
