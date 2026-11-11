# Trazas de prueba

Uso: `load workloads/<archivo>`. `ARRIVAL_TIME` es relativo al reloj actual.

| Archivo | Qué prueba | Resultado esperado |
|---|---|---|
| `workload_test.txt` | Traza del enunciado | Carga 3 procesos (PID 1 a 3), llegadas 0, 1 y 3 |
| `con_comentarios.txt` | Comentarios, líneas vacías, espacios y tabs | Igual que `workload_test.txt` |
| `invalido_1.txt` | Burst negativo, prioridad 9, solo 3 campos | Errores en línea 2 (BURST_TIME), 3 (PRIORIDAD) y 4 (campos); no se carga nada |
| `invalido_2.txt` | Texto en campo numérico, nombre con espacios | Errores en línea 2 (BURST_TIME) y 3 (nombre inválido); no se carga nada |
| `vacio.txt` | Archivo de 0 bytes | Aviso "no contiene procesos" |
| `solo_comentarios.txt` | Solo comentarios y líneas vacías | Aviso "no contiene procesos" |
| `hueco_idle.txt` | Un proceso que llega en el tick 3 | Gantt con `[0-3s: IDLE]` antes del proceso |
