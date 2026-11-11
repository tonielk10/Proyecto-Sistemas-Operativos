// RegistroGantt.h - Registro thread-safe del diagrama de Gantt (un segmento por tramo continuo).
// Dependencias: <pthread.h>, <string>, <vector>
#ifndef REGISTRO_GANTT_H
#define REGISTRO_GANTT_H

#include <pthread.h>
#include <string>
#include <vector>

// pid 0 y nombre "IDLE" representan CPU ociosa. El segmento es [inicio, fin).
struct SegmentoGantt {
    int inicio;
    int fin;
    int pid;
    std::string nombre;
};

class RegistroGantt {
public:
    RegistroGantt();
    ~RegistroGantt();

    RegistroGantt(const RegistroGantt&) = delete;
    RegistroGantt& operator=(const RegistroGantt&) = delete;

    /** Toma el mutex. Extiende el ultimo segmento si es el mismo pid y contiguo; si no, abre uno nuevo. */
    void registrar_tick(int tick, int pid, const std::string& nombre);
    /** Toma el mutex. Copia de los segmentos. */
    std::vector<SegmentoGantt> segmentos() const;

private:
    mutable pthread_mutex_t mutex_;
    std::vector<SegmentoGantt> segmentos_;
};

/** Funcion pura: texto del diagrama, sin salto de linea final. */
std::string formatearGantt(const std::vector<SegmentoGantt>& segs);

#endif
