// Pruebas de parsearTraza y del comando load (incluye el escenario 2 de punta a punta).
// Ejecutar desde la raiz del repo (usa rutas workloads/...).
// Compilar: FUENTES=$(grep -L "int main" src/*.cpp)
//   g++ -std=c++17 -Wall -Wextra -pthread -I. -Iinclude $FUENTES tests/test_load.cpp -o /tmp/test_load
#include "../include/Comandos.h"
#include "../include/Mensajes.h"
#include "../include/Metricas.h"
#include "../include/Motor.h"
#include "../include/RegistroGantt.h"
#include "../include/Traza.h"

#include <unistd.h>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#define CHECK(cond) do { if (!(cond)) { \
    std::cerr << "FALLO linea " << __LINE__ << ": " #cond "\n"; return 1; } } while (0)

namespace {

struct Entorno {
    EstadoSimulador estado;
    TablaProcesos tabla;
    ColaListos cola;
    ListaLlegadas llegadas;
    GestorProcesos gestor{estado, tabla, cola, llegadas};
    RegistroGantt gantt;
    ContextoCli ctx{estado, tabla, cola, llegadas, gestor, false};
    Motor motor{estado, tabla, cola, llegadas, gestor, gantt};
    Entorno() { ctx.gantt = &gantt; }
};

std::string cargar(Entorno& e, const std::vector<std::string>& args) {
    std::ostringstream buf;
    std::streambuf* viejo = std::cout.rdbuf(buf.rdbuf());
    comandoLoad(args, e.ctx);
    std::cout.rdbuf(viejo);
    return buf.str();
}

ResultadoTraza parsear(const std::string& texto) {
    std::istringstream in(texto);
    return parsearTraza(in);
}

bool contiene(const std::string& texto, const std::string& parte) {
    return texto.find(parte) != std::string::npos;
}

// true si hay un unico error en la linea dada con ese detalle
bool unError(const ResultadoTraza& r, int linea, const std::string& detalle) {
    return r.errores.size() == 1 && r.errores[0].linea == linea && r.errores[0].detalle == detalle &&
           r.entradas.empty();
}

PCB pcb_de(Entorno& e, int pid) {
    PCB p(0, "", 0, 0, 0);
    e.tabla.obtener(pid, p);
    return p;
}

bool metricas(Entorno& e, int pid, int espera, int retorno, int respuesta) {
    PCB p = pcb_de(e, pid);
    return p.estado == EstadoProceso::TERMINATED && p.tiempo_espera == espera &&
           p.tiempo_retorno == retorno && p.tiempo_respuesta == respuesta;
}

bool correr_hasta_el_final(Entorno& e) {
    e.motor.iniciar_hilo();
    e.estado.iniciar();
    for (int i = 0; i < 5000 && !e.estado.esta_pausado(); ++i) usleep(1000);
    bool ok = e.estado.esta_pausado();
    e.motor.detener_y_unir();
    return ok;
}

} // namespace

int main() {
    {
        ResultadoTraza r = parsear("Job_A, 4, 2, 0\nJob_B, 2, 1, 1\nJob_C, 1, 3, 3\n");
        CHECK(r.errores.empty() && r.entradas.size() == 3);
        CHECK(r.entradas[0].nombre == "Job_A" && r.entradas[0].burst == 4 && r.entradas[0].prioridad == 2 &&
              r.entradas[0].llegada == 0);
        CHECK(r.entradas[2].nombre == "Job_C" && r.entradas[2].burst == 1 && r.entradas[2].prioridad == 3 &&
              r.entradas[2].llegada == 3);
        std::cout << "OK parsearTraza tres lineas validas\n";
    }
    {
        ResultadoTraza r = parsear("# c\n\n   \n  # c2\n  A \t,\t 1 , 2 ,\t0\n\t\n B,2,3,4\n");
        CHECK(r.errores.empty() && r.entradas.size() == 2);
        CHECK(r.entradas[0].nombre == "A" && r.entradas[0].burst == 1 && r.entradas[1].llegada == 4);
        std::cout << "OK parsearTraza comentarios, vacias, espacios y tabs\n";
    }
    {
        ResultadoTraza r = parsear("A,1,1,0\r\nB,2,2,1\r\n");
        CHECK(r.errores.empty() && r.entradas.size() == 2 && r.entradas[1].nombre == "B" && r.entradas[1].llegada == 1);
        ResultadoTraza b = parsear("\xEF\xBB\xBF" "A,1,1,0\nB,2,2,1");
        CHECK(b.errores.empty() && b.entradas.size() == 2 && b.entradas[0].nombre == "A");
        CHECK(b.entradas[1].nombre == "B");  // sin salto final
        std::cout << "OK parsearTraza CRLF, BOM y sin salto final\n";
    }
    {
        CHECK(unError(parsear("A,1,1\n"), 1,
                      "Se esperaban 4 campos (NOMBRE, BURST_TIME, PRIORIDAD, ARRIVAL_TIME) y hay 3."));
        CHECK(unError(parsear("A,1,1,0,9\n"), 1,
                      "Se esperaban 4 campos (NOMBRE, BURST_TIME, PRIORIDAD, ARRIVAL_TIME) y hay 5."));
        CHECK(unError(parsear("a,,b\n"), 1,
                      "Se esperaban 4 campos (NOMBRE, BURST_TIME, PRIORIDAD, ARRIVAL_TIME) y hay 3."));
        CHECK(unError(parsear("A,,1,0\n"), 1, "BURST_TIME debe ser un número entero."));
        std::cout << "OK parsearTraza cantidad de campos\n";
    }
    {
        const std::string nom = "Nombre inválido (1 a 32 caracteres, sin espacios, comas ni caracteres de control).";
        CHECK(unError(parsear("Mal Nombre,1,1,0\n"), 1, nom));
        CHECK(unError(parsear(",1,1,0\n"), 1, nom));
        CHECK(unError(parsear(std::string(33, 'x') + ",1,1,0\n"), 1, nom));
        CHECK(unError(parsear("A,uno,1,0\n"), 1, "BURST_TIME debe ser un número entero."));
        CHECK(unError(parsear("A,0,1,0\n"), 1, "BURST_TIME debe estar entre 1 y 1000000."));
        CHECK(unError(parsear("A,-3,1,0\n"), 1, "BURST_TIME debe estar entre 1 y 1000000."));
        CHECK(unError(parsear("A,1,x,0\n"), 1, "PRIORIDAD debe ser un número entero."));
        CHECK(unError(parsear("A,1,9,0\n"), 1, "PRIORIDAD debe estar entre 1 y 5."));
        CHECK(unError(parsear("A,1,1,z\n"), 1, "ARRIVAL_TIME debe ser un número entero."));
        CHECK(unError(parsear("A,1,1,-1\n"), 1, "ARRIVAL_TIME debe estar entre 0 y 1000000."));
        CHECK(unError(parsear("A,1,1,1000001\n"), 1, "ARRIVAL_TIME debe estar entre 0 y 1000000."));
        // solo el primer error de la linea
        CHECK(unError(parsear("A,x,9,z\n"), 1, "BURST_TIME debe ser un número entero."));
        std::cout << "OK parsearTraza cada tipo de error\n";
    }
    {
        ResultadoTraza r = parsear("# cabecera\nA,1,1,0\nB,x,1,0\n\nC,1,1,0\nD,1,7,0\nE,1\n");
        CHECK(r.entradas.size() == 2);
        CHECK(r.errores.size() == 3);
        CHECK(r.errores[0].linea == 3 && r.errores[1].linea == 6 && r.errores[2].linea == 7);
        std::cout << "OK parsearTraza numeros de linea\n";
    }
    {
        CHECK(parsear("").entradas.empty() && parsear("").errores.empty());
        ResultadoTraza c = parsear("# a\n\n  # b\n");
        CHECK(c.entradas.empty() && c.errores.empty());
        std::cout << "OK parsearTraza vacio y solo comentarios\n";
    }
    {
        CHECK(unError(parsear(std::string(2000, 'a') + "\n"), 1, "Línea demasiado larga (máximo 1024 caracteres)."));
        CHECK(unError(parsear("A\x01" "B,1,1,0\n"), 1, "La línea contiene caracteres no válidos."));
        CHECK(unError(parsear("A\x7f,1,1,0\n"), 1, "La línea contiene caracteres no válidos."));
        std::cout << "OK parsearTraza linea larga y bytes de control\n";
    }
    {
        std::string texto;
        for (std::size_t i = 0; i < TRAZA_MAX_PROCESOS + 1; ++i) texto += "P,1,1,0\n";
        ResultadoTraza r = parsear(texto);
        CHECK(r.errores.size() == 1);
        CHECK(r.errores[0].linea == 10001);
        CHECK(r.errores[0].detalle == "Se superó el máximo de 10000 procesos por archivo.");
        ResultadoTraza ok = parsear(texto.substr(0, texto.size() - 8));  // exactamente 10000
        CHECK(ok.errores.empty() && ok.entradas.size() == 10000);
        std::cout << "OK parsearTraza maximo de procesos\n";
    }

    // comandoLoad con los archivos de workloads/
    const char* validos[] = {"workloads/workload_test.txt", "workloads/con_comentarios.txt"};
    for (const char* ruta : validos) {
        Entorno e;
        std::string out = cargar(e, {ruta});
        CHECK(contiene(out, "[OK] Se cargaron 3 proceso(s) desde '" + std::string(ruta) + "' (PID 1 a 3)."));
        CHECK(contiene(out, "[INFO] Usa 'start' para iniciar la simulación."));
        CHECK(e.tabla.tamano() == 3 && e.llegadas.tamano() == 3 && e.cola.vacia());
        const int esperadas[] = {0, 1, 3};
        const char* nombres[] = {"Job_A", "Job_B", "Job_C"};
        for (int i = 0; i < 3; ++i) {
            PCB p = pcb_de(e, i + 1);
            CHECK(p.estado == EstadoProceso::NEW && p.tiempo_llegada == esperadas[i] && p.nombre == nombres[i]);
        }
        Entorno f;
        for (int i = 0; i < 5; ++i) f.estado.avanzar_reloj();
        cargar(f, {ruta});
        CHECK(pcb_de(f, 1).tiempo_llegada == 5 && pcb_de(f, 2).tiempo_llegada == 6 && pcb_de(f, 3).tiempo_llegada == 8);
        CHECK(f.llegadas.tamano() == 3 && f.cola.vacia());
        std::cout << "OK comandoLoad " << ruta << "\n";
    }
    {
        Entorno e;
        std::string out = cargar(e, {"workloads/invalido_1.txt"});
        CHECK(contiene(out, "[ERROR]: Línea 2: BURST_TIME debe estar entre 1 y 1000000."));
        CHECK(contiene(out, "[ERROR]: Línea 3: PRIORIDAD debe estar entre 1 y 5."));
        CHECK(contiene(out, "[ERROR]: Línea 4: Se esperaban 4 campos"));
        CHECK(contiene(out, "tiene 3 línea(s) inválida(s): no se cargó ningún proceso."));
        CHECK(e.tabla.tamano() == 0 && e.llegadas.tamano() == 0 && e.cola.vacia());
        std::string out2 = cargar(e, {"workloads/invalido_2.txt"});
        CHECK(contiene(out2, "[ERROR]: Línea 2: BURST_TIME debe ser un número entero."));
        CHECK(contiene(out2, "[ERROR]: Línea 3: Nombre inválido"));
        CHECK(contiene(out2, "tiene 2 línea(s) inválida(s)"));
        CHECK(e.tabla.tamano() == 0 && e.llegadas.tamano() == 0);
        std::cout << "OK comandoLoad archivos invalidos (atomico)\n";
    }
    {
        Entorno e;
        CHECK(contiene(cargar(e, {"workloads/no_existe.txt"}),
                       "[ERROR DE PARAMETRO]: El archivo 'workloads/no_existe.txt' no existe."));
        CHECK(contiene(cargar(e, {"workloads"}), "[ERROR DE PARAMETRO]: 'workloads' no es un archivo regular."));
        CHECK(contiene(cargar(e, {"workloads/vacio.txt"}),
                       "[INFO] El archivo 'workloads/vacio.txt' no contiene procesos."));
        CHECK(contiene(cargar(e, {"workloads/solo_comentarios.txt"}),
                       "[INFO] El archivo 'workloads/solo_comentarios.txt' no contiene procesos."));
        std::string sin = cargar(e, {});
        CHECK(contiene(sin, "[ERROR DE SINTAXIS]: Parámetros insuficientes.") && contiene(sin, "load <archivo>"));
        CHECK(contiene(cargar(e, {"a", "b"}), "[ERROR DE SINTAXIS]: Demasiados parámetros."));
        CHECK(e.tabla.tamano() == 0 && e.llegadas.tamano() == 0);
        std::cout << "OK comandoLoad errores de ruta y argumentos\n";
    }
    {
        const std::string ruta = "/tmp/traza_enorme_test_load.txt";
        {
            std::ofstream f(ruta);
            for (int i = 0; i < 10001; ++i) f << "P" << i << ",1,1,0\n";
        }
        Entorno e;
        std::string out = cargar(e, {ruta});
        CHECK(contiene(out, "Se superó el máximo de 10000 procesos por archivo."));
        CHECK(contiene(out, "no se cargó ningún proceso."));
        CHECK(e.tabla.tamano() == 0 && e.llegadas.tamano() == 0);
        std::remove(ruta.c_str());
        std::cout << "OK comandoLoad archivo con mas del maximo\n";
    }
    {
        Entorno e;
        e.estado.set_velocidad(0);
        e.estado.set_politica(Politica::SRTF, 2);
        cargar(e, {"workloads/workload_test.txt"});
        CHECK(correr_hasta_el_final(e));
        std::vector<SegmentoGantt> g = e.gantt.segmentos();
        CHECK(formatearGantt(g) ==
              "DIAGRAMA DE GANTT (0 - 7 ticks):\n"
              "[0-1s: Job_A] -> [1-3s: Job_B] -> [3-4s: Job_C] -> [4-7s: Job_A]");
        CHECK(metricas(e, 1, 3, 7, 0));
        CHECK(metricas(e, 2, 0, 2, 0));
        CHECK(metricas(e, 3, 0, 1, 0));
        std::cout << "OK escenario 2 de punta a punta\n";
    }
    {
        Entorno e;
        e.estado.set_velocidad(0);
        e.estado.set_politica(Politica::RR, 2);
        cargar(e, {"workloads/hueco_idle.txt"});
        CHECK(correr_hasta_el_final(e));
        std::vector<SegmentoGantt> g = e.gantt.segmentos();
        CHECK(formatearGantt(g) == "DIAGRAMA DE GANTT (0 - 5 ticks):\n[0-3s: IDLE] -> [3-5s: X]");
        ResumenMetricas r = calcularResumen(e.tabla.listar(), g);
        CHECK(std::fabs(r.uso_cpu - 40.0) < 0.005);
        std::cout << "OK IDLE con hueco_idle.txt\n";
    }
    return 0;
}
