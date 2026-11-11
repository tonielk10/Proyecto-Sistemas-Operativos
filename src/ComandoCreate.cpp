#include "../include/ComandoCreate.h"
#include "../include/Mensajes.h"
#include "../include/ValidadorNombre.h"
#include "../include/ParserEntero.h" // Asegúrate de que este sea el nombre real de tu cabecera
#include <iostream>

void comandoCreate(const std::vector<std::string>& args, ContextoCli& ctx) {
    // 1. Validar la cantidad de argumentos (nombre y burst obligatorios, prioridad opcional)
    if (args.size() < 2 || args.size() > 3) {
        std::cout << Mensajes::errorSintaxis("Parametros insuficientes o excedidos para create.", Mensajes::USO_CREATE) << "\n";
        return;
    }

    std::string nombre = args[0];

    // 2. Validar el nombre del proceso usando el módulo ValidadorNombre
    if (!nombreValido(nombre)) {
        std::cout << Mensajes::errorTipo("El nombre del proceso no es valido.") << "\n";
        return;
    }

    // 3. Validar y parsear el Burst Time usando tu función real
    long burst_long = 0;
    // Asumiendo que el burst debe ser mayor a 0 (por ejemplo, hasta un límite razonable como 1000000)
    ResultadoParseo res_burst = parsearEntero(args[1], 1, 1000000, burst_long);
    if (res_burst != ResultadoParseo::OK) { // O la condición de éxito que use tu enumeración
        std::cout << Mensajes::errorTipo("El burst debe ser un numero entero positivo valido.") << "\n";
        return;
    }
    int burst_time = static_cast<int>(burst_long);

    // 4. Validar y parsear la Prioridad (opcional, por defecto 3, rango típico de 1 a 5)
    int prioridad = 3; 
    if (args.size() == 3) {
        long prio_long = 0;
        ResultadoParseo res_prio = parsearEntero(args[2], 1, 5, prio_long);
        if (res_prio != ResultadoParseo::OK) {
            std::cout << Mensajes::errorParametro("La prioridad debe ser un entero entre 1 y 5.") << "\n";
            return;
        }
        prioridad = static_cast<int>(prio_long);
    }

    // 5. Crear el proceso a través del GestorProcesos usando el contexto
    int nuevo_pid = ctx.gestor.crear_proceso(nombre, burst_time, prioridad);

    if (nuevo_pid > 0) {
        std::cout << Mensajes::mensajeOk("Proceso '" + nombre + "' creado exitosamente con PID " + std::to_string(nuevo_pid)) << "\n";
    } else {
        std::cout << Mensajes::errorGeneral("No se pudo crear el proceso en el sistema.") << "\n";
    }
}