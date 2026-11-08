// EstadoSimulador.h - Panel de control compartido entre el hilo CLI y el hilo Motor.
// Dependencias: <pthread.h>
//
// El mutex protege: clock_ticks_, ms_por_tick_, politica_, quantum_, pausado_ y apagar_.
// La variable de condicion permite al motor esperar (pausado) sin consumir CPU.
//
// Importante: el hilo motor debe haber sido unido (pthread_join) antes de destruir
// el objeto, pues el destructor destruye el mutex y la variable de condicion.
#ifndef ESTADO_SIMULADOR_H
#define ESTADO_SIMULADOR_H

#include <pthread.h>

enum class Politica { RR, SRTF };

inline const char* nombrePolitica(Politica p) {
    switch (p) {
        case Politica::RR:   return "RR";
        case Politica::SRTF: return "SRTF";
    }
    return "DESCONOCIDA";
}

// Foto consistente de lo que el motor necesita por tick.
struct ConfigTick {
    Politica politica;
    int quantum;
    int ms_por_tick;
};

class EstadoSimulador {
public:
    EstadoSimulador();
    ~EstadoSimulador();

    EstadoSimulador(const EstadoSimulador&) = delete;
    EstadoSimulador& operator=(const EstadoSimulador&) = delete;

    // --- Para el hilo CLI ---

    /** Hilo CLI. Toma el mutex. Devuelve false sin cambios si quantum <= 0. */
    bool set_politica(Politica p, int quantum);
    /** Hilo CLI. Toma el mutex. Devuelve false sin cambios si ms_por_tick < 0. */
    bool set_velocidad(int ms_por_tick);
    /** Hilo CLI. Toma el mutex. Reanuda el motor (pausado = false) y lo despierta. */
    void iniciar();
    /** Hilo CLI. Toma el mutex. El motor lo notara al inicio del siguiente tick. */
    void pausar();
    /** Hilo CLI. Toma el mutex. Marca el apagado y despierta al motor si esta esperando. */
    void pedir_apagado();

    // --- Para el hilo Motor ---

    /** Hilo Motor. Toma el mutex. Bloquea mientras este pausado; false si hay que terminar. */
    bool esperar_si_pausado();
    /** Hilo Motor. Toma el mutex. Copia politica, quantum y ms_por_tick de forma atomica. */
    ConfigTick leer_config() const;
    /** Hilo Motor. Toma el mutex. Incrementa el reloj simulado y devuelve el nuevo valor. */
    int avanzar_reloj();
    /** Cualquier hilo. Toma el mutex. */
    int leer_reloj() const;

    // --- Consulta (CLI y Motor) ---

    /** Cualquier hilo. Toma el mutex. */
    bool esta_pausado() const;
    /** Cualquier hilo. Toma el mutex. */
    bool apagado() const;

private:
    mutable pthread_mutex_t mutex_;
    pthread_cond_t cond_;

    int clock_ticks_;
    int ms_por_tick_;
    Politica politica_;
    int quantum_;
    bool pausado_;
    bool apagar_;
};

#endif
