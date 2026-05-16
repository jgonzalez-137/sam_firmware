---
name: create_sensor_task
description: |
  Skill para crear tasks FreeRTOS orientadas a adquisición periódica de sensores
  en el proyecto SAM. Provee plantilla, reglas obligatorias, checklist y ejemplos
  de frecuencias y manejo de errores para mantener tasks estables y ligeras.
scope: workspace
---

Objetivo

Crear tasks FreeRTOS que adquieran datos periódicamente, actualicen drivers,
publiquen eventos y mantengan timing estable sin ejecutar lógica de detección
o UI.

Responsabilidad de la task

- Adquirir datos del sensor.
- Llamar a `sensor.update()` o APIs del driver.
- Publicar eventos o escribir en una cola/structure compartida.
- Mantener timing estable.

NO permitido en la task

- Procesamiento complejo o detección de eventos.
- Control de UI o alertas.
- Bloqueos largos o `delay()`.

Reglas obligatorias

- Usar `vTaskDelay()` o `pdMS_TO_TICKS()` para el timing.
- No usar `delay()` ni bucles bloqueantes.
- Minimizar carga CPU dentro del loop.
- Manejar errores del driver sin provocar crash global.

Estructura recomendada (esqueleto)

```cpp
void sensorTask(void* parameter) {
    auto* sensor = static_cast<DriverName*>(parameter);
    const TickType_t period = pdMS_TO_TICKS(20); // ajustar según sensor

    while (true) {
        if (!sensor->isConnected()) {
            // reintentar o reportar (no bloquear)
        } else {
            sensor->update();
            // publicar lecturas en cola o event bus
        }

        vTaskDelay(period);
    }
}
```

Frecuencias sugeridas

- MPU6050: 50 Hz – 100 Hz (periodo 10–20 ms)
- MAX30102: 25 Hz – 50 Hz (periodo 20–40 ms)
- ADC/analog mic: 100 Hz o según necesidad de muestreo

Logging

- Solo logs importantes (init fail, desconexión prolongada, reintentos).
- No hacer spam serial cada ciclo.

Manejo de errores

- Si el sensor falla: reintentar con backoff incremental, reportar mediante evento,
  mantener la task viva y evitar crash global.
- Evitar bloqueos largos; delegar reconexiones a un scheduler o contador de reintentos.

Checklist antes de merge

- [ ] `vTaskDelay()` usado, no `delay()`
- [ ] Periodo/timing documentado en el archivo o README
- [ ] Logs mínimos implementados (init, fail, reconnect)
- [ ] La task publica eventos/colas en lugar de procesar detecciones
- [ ] Uso de pila estimado y validado para la task

Decisiones y variantes

- Para sensores de alta frecuencia, usar doble buffering y producir a una cola.
- Si la conversión es costosa, enviar datos brutos a `services/` para procesado.

Preguntas de aclaración (si aplica)

- ¿Deseas valores por defecto de periodo para cada sensor en un archivo de `config/`?
- ¿Prefieres que las tasks creen/gestión sus propias colas o que un `service` central
  las gestione?

Ejemplos de prompts para invocar esta skill

- "Create sensor task: scaffold FreeRTOS task for MPU6050 at 100Hz"
- "Create sensor task: implement MAX30102 sampling task with reconnect logic"

Siguientes personalizaciones sugeridas

- Generador de skeletons que cree `drivers/<name>/` + `tests/<name>_task.cpp`.
- `.instructions.md` con límites de stack y reglas de logging para tasks.

FIN
