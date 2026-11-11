#!/usr/bin/env bash
# Genera la evidencia de concurrencia y memoria del simulador.
#
# Uso (desde la raiz del repo):  bash scripts/generar_evidencia.sh
# Variables opcionales:
#   FACTOR_VG=4   multiplicador de las pausas para Valgrind/Helgrind (son mucho mas lentos)
#   FACTOR_SAN=1  multiplicador de las pausas para los binarios con sanitizers
#
# Ejecuta, con `make`, las pruebas (normal y TSan), el simulador real con TSan y ASan/UBSan
# (escenario1, escenario2, invalidos), y Valgrind memcheck y Helgrind sobre build/debug
# (escenario1, escenario2). Un .txt por ejecucion en docs/evidencia/, mas RESUMEN.md.
# Sigue aunque una ejecucion falle; termina con codigo != 0 si alguna tuvo reportes.

set -u
cd "$(dirname "$0")/.." || exit 1

DIR=docs/evidencia
FACTOR_VG=${FACTOR_VG:-4}
FACTOR_SAN=${FACTOR_SAN:-1}
FACTOR=1
mkdir -p "$DIR"

FECHA=$(date '+%Y-%m-%d %H:%M:%S')
COMMIT=$(git rev-parse --short HEAD 2>/dev/null || echo "?")
V_GCC=$(g++ --version | head -1)
V_VG=$(valgrind --version 2>/dev/null || echo "no disponible")

FILAS=()
TOTAL_REPORTES=0

# ---------- Entradas del simulador (stdin con pausas) ----------
esp() { sleep "$(awk -v s="$1" -v f="$FACTOR" 'BEGIN{print s*f}')"; }
cmd() { printf '%s\n' "$1"; esp "${2:-0.3}"; }

entrada_escenario1() {
    esp "${ARRANQUE:-0}"
    cmd "create BadProcess"
    cmd "pcb xyz"
    cmd "set_sched RR 2"
    cmd "set_speed 50"
    cmd "create P1 5 2"
    cmd "create P2 3 1"
    cmd "create P3 2 3"
    cmd "start" 0.4
    cmd "ps"
    cmd "pause"
    cmd "pcb 1"
    cmd "set_sched SRTF"
    cmd "start" 4
    cmd "ps"
    cmd "gantt"
    cmd "stats"
    cmd "exit" 0
}

entrada_escenario2() {
    esp "${ARRANQUE:-0}"
    cmd "set_sched SRTF"
    cmd "set_speed 50"
    cmd "load workloads/workload_test.txt"
    cmd "start" 4
    cmd "ps"
    cmd "gantt"
    cmd "stats"
    cmd "exit" 0
}

entrada_invalidos() {
    esp "${ARRANQUE:-0}"
    cmd "set_speed 50" 0.1
    cmd "create" 0.1
    cmd "create A" 0.1
    cmd "create A abc" 0.1
    cmd "create A 0" 0.1
    cmd "create A -3" 0.1
    cmd "create A 5 x" 0.1
    cmd "create A 5 2 extra" 0.1
    cmd "create ok1 6 1" 0.1
    cmd "start" 0.3
    cmd "create ok2 3 2" 0.1
    cmd "ps extra" 0.1
    cmd "pcb" 0.1
    cmd "pcb 1 2" 0.1
    cmd "pcb xyz" 0.1
    cmd "pcb 0" 0.1
    cmd "pcb 99999" 0.1
    cmd "set_sched" 0.1
    cmd "set_sched FIFO" 0.1
    cmd "set_sched RR" 0.1
    cmd "set_sched RR 0" 0.1
    cmd "set_sched RR x" 0.1
    cmd "set_sched SRTF 3" 0.1
    cmd "set_speed" 0.1
    cmd "set_speed 10" 0.1
    cmd "set_speed abc" 0.1
    cmd "set_speed 50 60" 0.1
    cmd "start extra" 0.1
    cmd "pause extra" 0.1
    cmd "gantt x" 0.1
    cmd "stats x" 0.1
    cmd "help x" 0.1
    cmd "exit x" 0.1
    cmd "EXIT" 0.1
    cmd "comando_inexistente" 0.1
    printf '\n\n   \n'
    printf '%03000d\n' 0 | tr '0' 'x'          # linea de 3000 caracteres
    esp 0.1
    printf 'create ctrl\001x 5\n'               # caracter de control
    esp 0.1
    cmd "load" 0.1
    cmd "load a b" 0.1
    cmd "load /ruta/que/no/existe.txt" 0.1
    cmd "load workloads" 0.1
    cmd "load workloads/invalido_1.txt" 0.1
    cmd "load workloads/invalido_2.txt" 0.1
    cmd "create ok3 2 3" 0.1
    cmd "start" 3
    cmd "ps"
    cmd "exit" 0
}

# ---------- Utilidades ----------
# ejecutar <archivo> <funcion_entrada|-> <comando...>
# Si la entrada es "-", el comando se ejecuta sin stdin especial.
ejecutar() {
    local log=$1 entrada=$2; shift 2
    {
        echo "Comando: $* $([ "$entrada" != - ] && echo "< $entrada")"
        echo "Fecha: $(date '+%Y-%m-%d %H:%M:%S')"
        echo "----------------------------------------"
    } > "$log"
    local rc
    if [ "$entrada" = - ]; then
        "$@" >> "$log" 2>&1 < /dev/null; rc=$?
    else
        "$entrada" | "$@" >> "$log" 2>&1; rc=${PIPESTATUS[1]}
    fi
    echo "----------------------------------------" >> "$log"
    echo "Codigo de salida: $rc" >> "$log"
    return $rc
}

cuenta() { local n; n=$(grep -cE -- "$1" "$2"); echo "${n:-0}"; }

# registrar <herramienta> <entrada> <rc> <log> <tipo>
registrar() {
    local herr=$1 ent=$2 rc=$3 log=$4 tipo=$5 rep=0 detalle
    case $tipo in
        tsan)
            rep=$(cuenta 'WARNING: ThreadSanitizer' "$log"); detalle="$rep TSan" ;;
        asan)
            local a u
            a=$(cuenta 'ERROR: (Address|Leak)Sanitizer' "$log")
            u=$(cuenta 'runtime error:' "$log")
            rep=$((a + u)); detalle="$a ASan/LSan, $u UBSan" ;;
        vg)
            rep=$(grep -o 'ERROR SUMMARY: [0-9]* errors' "$log" | tail -1 | grep -o '[0-9]*' | head -1)
            rep=${rep:-0}
            local heap="no"
            grep -q 'All heap blocks were freed' "$log" && heap="si"
            if [ "$herr" = "Helgrind" ]; then detalle="$rep errores"
            else detalle="$rep errores, heap liberado: $heap"; fi ;;
        tests)
            rep=$(( $(cuenta 'WARNING: ThreadSanitizer' "$log") + $(cuenta 'ERROR: AddressSanitizer' "$log") ))
            local f; f=$(cuenta '^FALLO' "$log")
            detalle="$rep sanitizer, $f pruebas FALLO" ;;
    esac
    local res=OK
    if [ "$rc" -ne 0 ] || [ "$rep" -ne 0 ]; then res=FALLO; fi
    TOTAL_REPORTES=$((TOTAL_REPORTES + rep))
    FILAS+=("| $herr | $ent | $res | $detalle | $FECHA | $COMMIT | \`${log#$DIR/}\` |")
    echo "[$res] $herr / $ent ($detalle)"
}

# ---------- Compilacion ----------
echo "== Compilando =="
make debug tsan asan > "$DIR/compilacion.txt" 2>&1 || { echo "Fallo la compilacion, ver $DIR/compilacion.txt"; exit 2; }
rm -f "$DIR/compilacion.txt"

# ---------- 1. Pruebas ----------
echo "== Pruebas =="
ejecutar "$DIR/tests_normal.txt" - make test; registrar "make test" "-" $? "$DIR/tests_normal.txt" tests
ejecutar "$DIR/tests_tsan.txt" - make test-tsan; registrar "make test-tsan" "-" $? "$DIR/tests_tsan.txt" tests

# ---------- 2. Simulador real con sanitizers ----------
echo "== Simulador real =="
FACTOR=$FACTOR_SAN; ARRANQUE=0
for ent in escenario1 escenario2 invalidos; do
    ejecutar "$DIR/sim_tsan_$ent.txt" "entrada_$ent" build/tsan/simulador
    registrar "TSan (simulador)" "$ent" $? "$DIR/sim_tsan_$ent.txt" tsan
    ejecutar "$DIR/sim_asan_$ent.txt" "entrada_$ent" build/asan/simulador
    registrar "ASan+UBSan (simulador)" "$ent" $? "$DIR/sim_asan_$ent.txt" asan
done

# ---------- 3. Valgrind ----------
echo "== Valgrind (lento) =="
FACTOR=$FACTOR_VG; ARRANQUE=$FACTOR_VG
for ent in escenario1 escenario2; do
    ejecutar "$DIR/valgrind_$ent.txt" "entrada_$ent" \
        valgrind --leak-check=full --error-exitcode=1 build/debug/simulador
    registrar "Valgrind memcheck" "$ent" $? "$DIR/valgrind_$ent.txt" vg
    ejecutar "$DIR/helgrind_$ent.txt" "entrada_$ent" \
        valgrind --tool=helgrind --error-exitcode=1 build/debug/simulador
    registrar "Helgrind" "$ent" $? "$DIR/helgrind_$ent.txt" vg
done

# ---------- 4. RESUMEN.md ----------
{
    echo "# Evidencia de concurrencia y memoria"
    echo
    echo "- Generado: $FECHA"
    echo "- Commit: \`$COMMIT\`"
    echo "- Compilador: $V_GCC"
    echo "- Valgrind: $V_VG"
    echo "- Reportes totales: $TOTAL_REPORTES"
    echo
    echo "| Herramienta | Entrada | Resultado | Reportes | Fecha | Commit | Log |"
    echo "|---|---|---|---|---|---|---|"
    printf '%s\n' "${FILAS[@]}"
    echo
    echo "Los reportes marcados como \`suppressed\` por Valgrind/Helgrind no se cuentan."
} > "$DIR/RESUMEN.md"

echo "Reportes totales: $TOTAL_REPORTES (ver $DIR/RESUMEN.md)"
[ "$TOTAL_REPORTES" -eq 0 ]
