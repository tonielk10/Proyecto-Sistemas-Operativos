// Cerrojo.h - Guard RAII para pthread_mutex_t (toma el lock al construir, lo libera al destruir).
// Dependencias: <pthread.h>
#ifndef CERROJO_H
#define CERROJO_H

#include <pthread.h>

class Cerrojo {
public:
    explicit Cerrojo(pthread_mutex_t& m) : mutex_(m) { pthread_mutex_lock(&mutex_); }
    ~Cerrojo() { pthread_mutex_unlock(&mutex_); }

    Cerrojo(const Cerrojo&) = delete;
    Cerrojo& operator=(const Cerrojo&) = delete;

private:
    pthread_mutex_t& mutex_;
};

#endif
