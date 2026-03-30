# SAM Wearable – Firmware & System Specification (Expanded)

## 1. Overview

SAM es un wearable de monitoreo pasivo orientado a detección temprana de eventos (caídas, anomalías cardíacas e inactividad) con consumo energético mínimo. Este documento define comportamiento, arquitectura y contratos necesarios para implementación directa en ESP32.

El sistema está diseñado bajo tres principios: determinismo (FSM), eficiencia energética y trazabilidad de decisiones (scoring).

---

## 2. Arquitectura interna del firmware

El firmware se divide en módulos desacoplados comunicados por un bus de eventos ligero.

Estructura sugerida de carpetas:

```
/core
  fsm.c
  event_bus.c
  scheduler.c
/sensors
  imu.c
  ppg.c
/detection
  features.c
  scoring.c
  thresholds.c
/ble
  ble_service.c
  ble_protocol.c
/system
  power.c
  persistence.c
/ui
  ui_controller.c
```

Cada módulo expone una interfaz mínima. No hay llamadas cruzadas directas entre sensores, BLE y UI; todo fluye mediante eventos.

---

## 3. Scheduler y ejecución

Modelo basado en FreeRTOS con tareas cooperativas.

Tareas:

- SensorTask (alta frecuencia)
- DetectionTask (media)
- SystemTask (control FSM)
- BLETask (comunicación)
- UITask (baja prioridad)

Pseudocódigo base:

```c
void app_main() {
  init_system();
  xTaskCreate(sensor_task, "sensor", ...);
  xTaskCreate(detection_task, "detect", ...);
  xTaskCreate(system_task, "system", ...);
  xTaskCreate(ble_task, "ble", ...);
  xTaskCreate(ui_task, "ui", ...);
}
```

---

## 4. Event Bus

Sistema central de comunicación.

```c
typedef enum {
  EV_SAMPLE_READY,
  EV_EVENT_DETECTED,
  EV_BLE_CONNECTED,
  EV_TIMEOUT,
  EV_ERROR
} event_t;

void publish(event_t ev, void* data);
void subscribe(event_t ev, handler_fn fn);
```

---

## 5. FSM del sistema (formal)

Estados: BOOT, INIT, CALIBRATION, NORMAL, LOW_POWER, EVENT, ERROR.

Pseudocódigo de transición:

```c
void fsm_step(event_t ev) {
  switch(state) {
    case BOOT:
      state = INIT;
      break;

    case INIT:
      if (ev == EV_INIT_OK) state = CALIBRATION;
      if (ev == EV_ERROR) state = ERROR;
      break;

    case CALIBRATION:
      if (ev == EV_CALIB_DONE) state = NORMAL;
      break;

    case NORMAL:
      if (ev == EV_INACTIVITY) state = LOW_POWER;
      if (ev == EV_EVENT_DETECTED) state = EVENT;
      break;

    case LOW_POWER:
      if (ev == EV_ACTIVITY) state = NORMAL;
      if (ev == EV_EVENT_DETECTED) state = EVENT;
      break;

    case EVENT:
      if (ev == EV_EVENT_HANDLED) state = NORMAL;
      break;

    case ERROR:
      if (ev == EV_RECOVERED) state = NORMAL;
      break;
  }
}
```

---

## 6. Adquisición y buffers

Buffers circulares por sensor.

```c
#define BUF_SIZE 128

typedef struct {
  float ax, ay, az;
  float gx, gy, gz;
} imu_sample_t;

imu_sample_t imu_buffer[BUF_SIZE];
int idx = 0;

void add_sample(imu_sample_t s) {
  imu_buffer[idx++] = s;
  if (idx >= BUF_SIZE) idx = 0;
}
```

Ventana de análisis típica: 1–3 segundos.

---

## 7. Extracción de features

```c
float acc_mag(sample) {
  return sqrt(sample.ax*sample.ax + sample.ay*sample.ay + sample.az*sample.az);
}

float gyro_mag(sample) {
  return sqrt(sample.gx*sample.gx + sample.gy*sample.gy + sample.gz*sample.gz);
}
```

---

## 8. Detección de caída (pipeline)

```c
if (acc_mag > IMPACT_THR) {
  if (gyro_mag > GYRO_THR && within_window(300)) {
    if (detect_immobility(5s)) {
      publish(EV_EVENT_DETECTED, FALL);
    }
  }
}
```

---

## 9. Sistema de scoring

```c
float compute_score(features f) {
  return w1*f.impact + w2*f.gyro + w3*f.sequence + w4*f.immob;
}

if (score > 0.8) trigger_event();
```

---

## 10. Umbrales adaptativos

```c
baseline = (1 - alpha) * baseline + alpha * new_value;
```

Los umbrales se recalculan dinámicamente en cada ciclo de detección.

---

## 11. BLE (flujo)

Estados: ADVERTISING → CONNECTED → SYNC → STREAMING.

```c
if (connected) {
  send_telemetry();
}
```

Paquetes binarios compactos.

---

## 12. Persistencia (NVS)

```c
nvs_set_blob("profile", &profile, sizeof(profile));
```

Se escribe solo en cambios significativos.

---

## 13. Gestión de energía

Reglas:

- bajar frecuencia en LOW_POWER
- apagar sensores no críticos

```c
if (state == LOW_POWER) {
  set_sampling_rate(LOW);
}
```

---

## 14. UI Controller

La UI no accede a sensores.

```c
ui_state = get_cached_state();
render(ui_state);
```

---

## 15. Interacción de botones

- ↑: navegación arriba
- ↓: navegación abajo
- central tap: seleccionar
- hold medio: asistente
- hold largo: acción crítica / volver HOME

---

## 16. Manejo de errores

```c
if (sensor_fail) {
  disable_sensor();
  publish(EV_ERROR, SENSOR);
}
```

---

## 17. Motor de detección avanzado (Signal Processing + Optimización)

El núcleo del sistema depende de un procesamiento eficiente y robusto de señales. Aquí se define una implementación viable en ESP32 sin uso de librerías pesadas.

### 17.1 Ventana deslizante (Sliding Window)

Se utiliza una ventana temporal fija con actualización incremental.

```c
#define WINDOW_SIZE 50

float acc_window[WINDOW_SIZE];
int w_idx = 0;

void update_window(float new_val) {
  acc_window[w_idx++] = new_val;
  if (w_idx >= WINDOW_SIZE) w_idx = 0;
}
```

No se recalculan métricas completas cada ciclo, solo actualizaciones incrementales.

---

### 17.2 Media y varianza optimizadas

```c
float mean = 0;
float variance = 0;

void update_stats(float new_val) {
  static float sum = 0;
  static float sum_sq = 0;

  sum += new_val;
  sum_sq += new_val * new_val;

  mean = sum / WINDOW_SIZE;
  variance = (sum_sq / WINDOW_SIZE) - (mean * mean);
}
```

Esto evita recorrer todo el buffer en cada iteración.

---

### 17.3 Filtro de ruido (Low-pass simple)

```c
float low_pass(float prev, float current, float alpha) {
  return alpha * current + (1 - alpha) * prev;
}
```

Aplicado a aceleración y PPG.

---

### 17.4 Normalización de señal

```c
float normalize(float value, float min, float max) {
  return (value - min) / (max - min);
}
```

Se utiliza antes del scoring.

---

### 17.5 Pipeline completo

```c
sample = read_imu();
acc = acc_mag(sample);
acc_filtered = low_pass(prev_acc, acc, 0.2);

update_window(acc_filtered);
update_stats(acc_filtered);

features.impact = acc_filtered;
features.variance = variance;
features.motion = gyro_mag(sample);

score = compute_score(features);

if (score > THRESHOLD) {
  publish(EV_EVENT_DETECTED, FALL);
}
```

---

## 18. Gestión avanzada de energía

El consumo define la viabilidad del producto.

### 18.1 Estados de consumo

- ACTIVE (~80-120 mA)
- IDLE (~20-40 mA)
- LOW_POWER (~5-10 mA)

---

### 18.2 Control dinámico de sensores

```c
if (state == LOW_POWER) {
  imu_set_rate(10);
  ppg_disable();
}

if (state == NORMAL) {
  imu_set_rate(50);
  ppg_enable();
}
```

---

### 18.3 Light Sleep

```c
esp_light_sleep_start();
```

Se activa cuando no hay eventos y no hay conexión BLE activa.

---

## 19. BLE – Definición binaria exacta

### 19.1 Paquete de telemetría

```c
typedef struct {
  uint8_t type;
  uint32_t timestamp;
  int16_t acc;
  int16_t gyro;
  uint8_t hr;
  uint8_t flags;
} __attribute__((packed)) packet_t;
```

---

### 19.2 Flujo de conexión

```c
on_connect() {
  state = CONNECTED;
}

on_disconnect() {
  state = ADVERTISING;
}
```

---

## 20. Persistencia robusta

### 20.1 Estructura versionada

```c
typedef struct {
  uint8_t version;
  float baseline_hr;
  float baseline_motion;
} profile_t;
```

---

### 20.2 Validación

```c
if (profile.version != CURRENT_VERSION) {
  reset_profile();
}
```

---

## 21. Sistema de errores avanzado

### 21.1 Watchdog

```c
esp_task_wdt_reset();
```

---

### 21.2 Recuperación

```c
if (sensor_fail) {
  reinit_sensor();
}
```

---

## 22. Integración UI ↔ FSM

La UI consume estados sin bloquear.

```c
ui_update(get_state_snapshot());
```

Eventos críticos interrumpen cualquier pantalla.

---

## 23. Sistema de eventos y flujo de alertas (SAM)

Este módulo define cómo un evento detectado se convierte en una alerta real. Es el componente más crítico a nivel producto, ya que controla falsos positivos, tiempos de respuesta y escalamiento.

---

### 23.1 Tipos de eventos

```c
typedef enum {
  EVENT_FALL,
  EVENT_ANOMALY_HR,
  EVENT_INACTIVITY,
  EVENT_MANUAL_TRIGGER
} event_type_t;
```

---

### 23.2 Estados del evento

Cada evento pasa por un ciclo controlado independiente del FSM principal.

Estados:

- DETECTED
- VALIDATING
- CONFIRMED
- USER_CANCELLED
- ESCALATED
- CLOSED

---

### 23.3 Máquina de estados del evento

```c
void event_fsm_step(event_t ev) {
  switch(event_state) {

    case DETECTED:
      start_timer(VALIDATION_TIME);
      event_state = VALIDATING;
      break;

    case VALIDATING:
      if (user_cancelled()) {
        event_state = USER_CANCELLED;
      }
      else if (timer_expired()) {
        event_state = CONFIRMED;
      }
      break;

    case CONFIRMED:
      trigger_alert();
      event_state = ESCALATED;
      break;

    case ESCALATED:
      if (ack_received()) {
        event_state = CLOSED;
      }
      break;

    case USER_CANCELLED:
      log_event(false_positive);
      event_state = CLOSED;
      break;

    case CLOSED:
      reset_event();
      break;
  }
}
```

---

### 23.4 Validación y reducción de falsos positivos

Reglas clave:

- Confirmación por ventana temporal
- Re-evaluación del score durante VALIDATING
- Cancelación manual prioritaria

```c
if (event_state == VALIDATING) {
  score = recompute_score();
  if (score < CANCEL_THR) {
    event_state = USER_CANCELLED;
  }
}
```

---

### 23.5 Temporizadores críticos

Valores sugeridos:

- VALIDATION_TIME: 10–20 segundos
- ESCALATION_DELAY: inmediato tras confirmación

```c
start_timer(15000); // 15 segundos
```

---

### 23.6 Flujo de alerta completo

1. Detección (FSM principal)
2. Evento entra en DETECTED
3. Se inicia VALIDATING
4. Usuario puede cancelar
5. Si no cancela → CONFIRMED
6. Se envía alerta por BLE
7. App escala (notificación / contacto)

---

### 23.7 Comunicación con la app

```c
typedef struct {
  uint8_t type;
  uint8_t event;
  uint8_t state;
  uint32_t timestamp;
} alert_packet_t;
```

---

### 23.8 Cancelación desde el dispositivo

```c
if (button_hold_long()) {
  cancel_event();
}
```

---

### 23.9 Manejo sin conexión

Si no hay conexión BLE:

- almacenar evento en buffer local
- reintentar envío
- activar feedback local (vibración / LED)

```c
if (!ble_connected) {
  store_event_locally();
}
```

---

### 23.10 Estrategia anti-falsos positivos

- scoring dinámico
- validación temporal
- confirmación implícita por inmovilidad
- cancelación manual

---

### 23.11 Feedback al usuario

Estados visibles:

- VALIDATING → vibración leve periódica
- CONFIRMED → vibración fuerte
- CANCELLED → vibración corta única

```c
vibrate(pattern);
```

---

### 23.12 Integración con FSM principal

Cuando ocurre un evento:

```c
publish(EV_EVENT_DETECTED, type);
```

FSM cambia a estado EVENT, pero el flujo real lo controla el event FSM.

---

### 23.13 Persistencia de eventos

Eventos críticos se almacenan:

```c
save_event_log(event);
```

---

## 24. Testing, calibración y métricas del sistema

Este bloque define cómo validar, ajustar y evolucionar el sistema en condiciones reales. Sin esta fase, el sistema no puede considerarse confiable.

---

### 24.1 Estrategia de testing

Se divide en tres niveles:

1. Unit testing (funciones individuales)
2. Integration testing (pipeline completo)
3. Field testing (uso real)

---

### 24.2 Testing de detección de caídas

Simulación controlada:

- caídas hacia adelante
- caídas laterales
- caídas con amortiguación (cama, sofá)

También se deben simular falsos positivos:

- sentarse bruscamente
- dejar caer el dispositivo
- movimientos deportivos

---

### 24.3 Registro de datos (Data Logging)

```c
log_sample(acc, gyro, hr, timestamp);
```

Los datos deben exportarse para análisis externo.

---

### 24.4 Dataset mínimo

Para calibración inicial:

- 50+ eventos reales
- 200+ eventos normales

---

### 24.5 Métricas clave

```c
precision = TP / (TP + FP);
recall = TP / (TP + FN);
```

Donde:

- TP: detecciones correctas
- FP: falsos positivos
- FN: eventos no detectados

---

### 24.6 Ajuste de thresholds

```c
if (FP > limite) {
  aumentar_threshold();
}

if (FN > limite) {
  disminuir_threshold();
}
```

El ajuste debe ser iterativo.

---

### 24.7 Validación del scoring

Se deben analizar distribuciones:

- score eventos reales vs normales

Objetivo:

- separación clara entre clases

---

### 24.8 Simulación offline

Reproducir datos grabados:

```c
for sample in dataset:
  process(sample);
```

Permite iterar sin hardware activo.

---

### 24.9 Pruebas de estrés

- batería baja
- pérdida de BLE
- múltiples eventos consecutivos

---

### 24.10 Testing del sistema de alertas

Verificar:

- tiempos de validación
- cancelación
- escalamiento

---

### 24.11 Métricas de sistema

- consumo promedio (mA)
- tiempo activo vs sleep
- latencia de detección

---

### 24.12 Iteración continua

Proceso:

1. recolectar datos
2. ajustar parámetros
3. re-test
4. desplegar

---

### 24.13 Preparación para producción

Checklist:

- falsos positivos aceptables
- latencia < 2s
- autonomía validada
- reconexión BLE estable

---

## 25. Conclusión final

El sistema SAM queda definido como una arquitectura completa, modular y lista para implementación real. Integra adquisición de datos, procesamiento eficiente, detección robusta, validación de eventos, comunicación BLE y un sistema de alertas confiable.

La calidad final dependerá directamente de la fase de testing y calibración.


Este sistema define un firmware modular, determinista y eficiente. Cada componente está desacoplado, lo que permite escalabilidad futura (OTA, ML).

El siguiente paso es implementación directa siguiendo esta estructura.

