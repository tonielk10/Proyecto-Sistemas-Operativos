#include "../include/Traza.h"
#include "../include/ParserEntero.h"
#include "../include/ValidadorNombre.h"

namespace {

std::string recortar(const std::string& s) {
    const char* blancos = " \t";
    std::size_t ini = s.find_first_not_of(blancos);
    if (ini == std::string::npos) return "";
    std::size_t fin = s.find_last_not_of(blancos);
    return s.substr(ini, fin - ini + 1);
}

// Separa por comas conservando los campos vacios ("a,,b" son 3 campos).
std::vector<std::string> separarCampos(const std::string& linea) {
    std::vector<std::string> campos;
    std::size_t ini = 0;
    while (true) {
        std::size_t coma = linea.find(',', ini);
        if (coma == std::string::npos) {
            campos.push_back(recortar(linea.substr(ini)));
            break;
        }
        campos.push_back(recortar(linea.substr(ini, coma - ini)));
        ini = coma + 1;
    }
    return campos;
}

bool tieneControl(const std::string& s) {
    for (unsigned char c : s) {
        if ((c < 32 && c != '\t') || c == 127) return true;
    }
    return false;
}

// Devuelve "" si el campo es valido; si no, el detalle del error.
std::string leerNumero(const std::string& texto, long min, long max, const std::string& etiqueta, int& salida) {
    long valor = 0;
    ResultadoParseo r = parsearEntero(texto, min, max, valor);
    if (r == ResultadoParseo::NO_ENTERO) return etiqueta + " debe ser un número entero.";
    if (r == ResultadoParseo::FUERA_DE_RANGO) {
        return etiqueta + " debe estar entre " + std::to_string(min) + " y " + std::to_string(max) + ".";
    }
    salida = static_cast<int>(valor);
    return "";
}

} // namespace

ResultadoTraza parsearTraza(std::istream& entrada) {
    ResultadoTraza res;
    std::string linea;
    int numero = 0;

    while (std::getline(entrada, linea)) {
        ++numero;
        if (numero == 1 && linea.compare(0, 3, "\xEF\xBB\xBF") == 0) linea.erase(0, 3);
        if (!linea.empty() && linea.back() == '\r') linea.pop_back();

        if (linea.size() > TRAZA_MAX_LINEA) {
            res.errores.push_back({numero, "Línea demasiado larga (máximo 1024 caracteres)."});
            continue;
        }
        if (tieneControl(linea)) {
            res.errores.push_back({numero, "La línea contiene caracteres no válidos."});
            continue;
        }

        linea = recortar(linea);
        if (linea.empty() || linea[0] == '#') continue;

        std::vector<std::string> campos = separarCampos(linea);
        if (campos.size() != 4) {
            res.errores.push_back({numero, "Se esperaban 4 campos (NOMBRE, BURST_TIME, PRIORIDAD, ARRIVAL_TIME) y hay " +
                                               std::to_string(campos.size()) + "."});
            continue;
        }

        EntradaTraza e{campos[0], 0, 0, 0};
        if (!nombreValido(e.nombre)) {
            res.errores.push_back({numero, "Nombre inválido (1 a 32 caracteres, sin espacios, comas ni caracteres de control)."});
            continue;
        }
        std::string err = leerNumero(campos[1], 1, 1000000, "BURST_TIME", e.burst);
        if (err.empty()) err = leerNumero(campos[2], 1, 5, "PRIORIDAD", e.prioridad);
        if (err.empty()) err = leerNumero(campos[3], 0, 1000000, "ARRIVAL_TIME", e.llegada);
        if (!err.empty()) {
            res.errores.push_back({numero, err});
            continue;
        }

        if (res.entradas.size() >= TRAZA_MAX_PROCESOS) {
            res.errores.push_back({numero, "Se superó el máximo de 10000 procesos por archivo."});
            break;
        }
        res.entradas.push_back(e);
    }
    return res;
}
