---
name: sam-module-implementation
description: 'Implementa modulos del firmware SAM en ESP32 con ESP-IDF y PlatformIO. Usar para sensores, detection, core (FSM/event bus), BLE, system y UI respetando arquitectura modular, pipeline, FSM, eficiencia energetica y reglas de produccion.'
argument-hint: 'Describe el modulo, evento(s) involucrado(s), archivo(s) objetivo y resultado esperado sin cambiar contratos de arquitectura.'
user-invocable: true
disable-model-invocation: false
---

# SAM Module Implementation

## Objetivo
Implementar modulos funcionales del firmware SAM listos para produccion, con integracion correcta al event bus y compatibilidad con FSM/pipeline, sin romper arquitectura ni contratos existentes.

## Fuente de verdad obligatoria
Antes de proponer cambios, validar reglas contra:
- docs/general_guide_to_sam_firmware.md
- docs/sam_wearable_firmware_specification.md
- docs/sam_physical_conections.md
- docs/structure.md

Si hay contradiccion o ambiguedad, proponer parche seguro por defecto con supuestos explicitos. Pedir confirmacion solo cuando el riesgo de romper contratos sea alto.

## Cuando usar esta skill
- Implementar o extender modulos en capas sensors, detection, core, ble, system o ui.
- Agregar eventos al event bus.
- Integrar scoring/features con FSM.
- Crear servicios BLE orientados a eventos.
- Aplicar optimizaciones de energia en tareas FreeRTOS.

## No usar esta skill para
- Cambiar arquitectura base del sistema.
- Introducir frameworks tipo Arduino.
- Hacer streaming BLE continuo en produccion.
- Tomar decisiones con datos crudos.

## Restricciones tecnicas
- Lenguaje: C (se permite C++ controlado solo por compatibilidad del proyecto, sin alterar contratos).
- Framework: ESP-IDF.
- Entorno: PlatformIO.
- RTOS: FreeRTOS.
- Sin delays bloqueantes; usar timers, eventos o mecanismos no bloqueantes.
- Minimizar CPU y memoria; evitar malloc frecuente; preferir buffers circulares.
- Validar errores en capas criticas.

## Contratos no negociables
- Comunicacion entre modulos solo por event bus.
- Sin acceso directo entre modulos fuera de interfaces publicas.
- FSM determinista y basada en eventos.
- FSM no accede sensores ni calcula features.
- Pipeline obligatorio y en orden:
  1. sensores
  2. procesamiento
  3. features
  4. scoring
  5. FSM
  6. evento
  7. BLE

## Flujo de ejecucion
1. Clasificar solicitud
- Identificar modulo objetivo: sensor, detection, FSM/event bus, BLE, system o UI.
- Determinar si el cambio es nuevo modulo, extension o correccion.

2. Ubicar rol en pipeline
- Definir entrada/salida del modulo y etapa exacta que ocupa.
- Confirmar que no adelanta decisiones con datos crudos.

3. Definir interfaz publica
- Especificar API minima (init, update/process, publish/consume event, teardown si aplica).
- Declarar structs y enums necesarios para datos y eventos.

4. Implementar logica minima correcta
- Mantener separacion adquisicion/procesamiento/decision.
- Usar procesamiento incremental y buffers circulares cuando aplique.
- Incluir manejo de errores y codigos de retorno consistentes.

5. Integrar con event bus
- Publicar eventos del modulo con payload compacto.
- Suscribir solo eventos necesarios.
- Evitar acoplamiento directo con otros modulos.

6. Compatibilidad FreeRTOS
- Integrar en tarea correspondiente (SensorTask, DetectionTask, SystemTask, BLETask, UITask).
- Verificar no bloqueo y comportamiento determinista.

7. Verificacion de calidad
- Validar compilacion ESP-IDF.
- Revisar consumo basico (CPU/memoria/radio) y bucles de alta frecuencia.
- Confirmar que no rompe contratos FSM/pipeline/BLE.

## Branching y decisiones
- Si falta dato de hardware o pinout: leer documentacion y aplicar defaults seguros; marcar supuesto.
- Si una solicitud rompe arquitectura: rechazar el cambio y proponer alternativa compatible.
- Si hay dos opciones validas: elegir la de menor acoplamiento y menor costo energetico.

## Criterios de completitud
Una implementacion se considera terminada solo si:
- Compila para ESP-IDF en el proyecto cuando sea posible; si no es posible compilar en ese momento, incluir validacion estatica explicita y limitaciones.
- Respeta arquitectura modular y event bus.
- Conserva orden del pipeline y reglas de FSM.
- No introduce acoplamiento indebido.
- Es eficiente en tiempo y memoria.
- Incluye archivos y puntos de integracion claros.

## Errores comunes a evitar
- Logica de negocio dentro de drivers.
- Acceso directo entre modulos.
- Memoria dinamica excesiva.
- Calculos innecesarios en loops de alta frecuencia.
- Manejo de errores incompleto.
- Saltarse etapas del pipeline.

## Formato de salida requerido
- Explicacion breve (maximo 5 lineas).
- Codigo completo listo para usar; permitir diffs cuando el cambio sea pequeno y mas claro que un volcado completo.
- Ubicacion exacta del archivo en el proyecto.
- Integracion con modulos/eventos/FSM.
- Verificacion realizada y riesgo residual.
