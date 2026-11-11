#!/bin/bash

# 1. Compilar el simulador según las especificaciones
g++ -std=c++17 -Wall -Wextra -pthread -I. -Iinclude src/*.cpp -o /tmp/simulador

if [ $? -ne 0 ]; then
    echo "Error de compilación."
    exit 1
fi

# 2. Ejecutar con la entrada básica y guardar la salida
/tmp/simulador < tests/entradas/consola_basica.txt > /tmp/salida_consola.txt

# 3. Comparar con diff
diff -u tests/esperado/consola_basica.txt /tmp/salida_consola.txt
DIFF_RESULT=$?

# 4. Evaluar el resultado final
if [ $DIFF_RESULT -eq 0 ]; then
    echo "OK"
    rm -f /tmp/salida_consola.txt
    exit 0
else
    echo "Las salidas no coinciden."
    rm -f /tmp/salida_consola.txt
    exit 1
fi