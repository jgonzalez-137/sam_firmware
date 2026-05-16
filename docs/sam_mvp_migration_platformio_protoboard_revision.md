# SAM Wearable MVP — Revisión de Arquitectura para Desarrollo en Protoboard + PlatformIO

## Objetivo de esta revisión

Esta revisión redefine oficialmente el estado actual del proyecto SAM.

El proyecto deja temporalmente el enfoque centrado en PCB personalizada y pasa a una arquitectura de desarrollo basada en:

- ESP32 DevKit V1 sobre protoboard
- alimentación USB directa desde computadora
- desarrollo completo mediante PlatformIO
- comunicación serial por cable
- conexión exclusivamente mediante USB-C
- validación funcional antes de fabricación final

La prioridad actual del proyecto es:

- desarrollo rápido de firmware
- validación real de sensores
- pruebas del pipeline de detección
- estabilización de arquitectura modular
- validación de interfaces
- pruebas seriales
- construcción de MVP funcional

No se busca todavía:

- miniaturización
- consumo energético final
- validación clínica
- optimización extrema
- encapsulado final
- autonomía real de batería

---

# Estado Oficial Actual del Hardware

## Configuración activa del sistema

La configuración oficial de desarrollo queda definida como:

| Elemento | Estado |
|---|---|
| MCU principal | ESP32 DevKit V1 USB-C |
| Programación | USB directo |
| Alimentación | USB desde laptop |
| Entorno firmware | PlatformIO |
| Framework principal | Arduino Framework sobre ESP32 |
| Comunicación externa | USB serial |
| PCB personalizada | Suspendida temporalmente |
| Plataforma de pruebas | Protoboard 830 puntos |

---

# Filosofía de Desarrollo Actual

SAM pasa oficialmente a una etapa de validación funcional iterativa.

Esto implica:

- firmware primero
- estabilidad primero
- modularidad primero
- pruebas reales antes de integración física final

La protoboard deja de considerarse una solución temporal improvisada y pasa a ser:

una plataforma oficial de desarrollo del MVP.

Esto afecta directamente:

- arquitectura firmware
- manejo energético
- documentación
- prioridades técnicas
- debugging
- integración serial
- estrategia de validación

---

# Cambio de Prioridades del Proyecto

## Antes

Prioridad:

- PCB
- miniaturización
- integración física
- autonomía portátil

## Ahora

Prioridad:

- firmware modular
- validación de sensores
- estabilidad I2C
- estabilidad SPI
- FSM
- BLE
- flujo de alertas
- integración aplicación móvil
- depuración completa

---

# Entorno Oficial de Desarrollo

## IDE Principal

El entorno oficial del proyecto pasa a ser:

PlatformIO.

Razones:

- soporte profesional ESP32
- estructura modular real
- integración FreeRTOS
- manejo avanzado de librerías
- serial monitor integrado
- escalabilidad del firmware
- mejor organización del proyecto
- integración futura con testing automatizado

---

# Configuración Oficial de Framework

## Framework principal

Arduino Framework sobre ESP32.

Razones:

- aceleración del desarrollo MVP
- ecosistema maduro de librerías
- integración rápida de sensores
- menor complejidad inicial
- mejor velocidad de iteración

ESP-IDF puro queda reservado para:

- optimización futura
- revisión avanzada de consumo
- revisión RTOS avanzada
- producción futura

---

# Estructura Oficial del Proyecto Firmware

La estructura modular queda oficialmente definida así:

```text
firmware/
│
├── core/
├── drivers/
├── services/
├── communication/
├── ui/
├── power/
├── configs/
├── tests/
└── main/
```

Separación funcional:

| Módulo | Responsabilidad |
|---|---|
| core | FSM, scheduler, event engine |
| drivers | acceso hardware |
| services | lógica de procesamiento |
| communication | BLE y telemetría |
| ui | pantalla y navegación |
| power | estados energéticos |
| configs | GPIO y thresholds |
| tests | validación hardware |
| main | inicialización principal |

---

# Estado Oficial de Alimentación

## Alimentación actual

Durante toda la etapa MVP sobre protoboard:

el sistema será alimentado directamente desde el puerto USB del ESP32.

Esto elimina temporalmente:

- batería LiPo
- TP4056
- LM1117
- validación energética avanzada
- carga portátil
- autonomía real

Razón:

La prioridad actual es estabilidad de desarrollo y debugging.

---

# Consecuencias Técnicas del Cambio de Alimentación

## Ventajas

- alimentación estable
- reducción de ruido eléctrico
- menos fallos de alimentación
- debugging más confiable
- flashing constante sin reinicio externo
- menor complejidad de montaje

## Restricciones

- no existen pruebas reales de batería
- no existen pruebas reales de autonomía
- gestión energética queda parcialmente simulada
- consumo energético aún no es representativo del producto final

---

# Política Oficial de Comunicación

## Comunicación primaria durante desarrollo

USB serial.

Uso:

- flashing
- logs
- debugging
- monitoreo
- calibración
- trazabilidad del pipeline

---

## Comunicación externa oficial

Toda comunicación externa del sistema durante el MVP se realizará exclusivamente mediante USB serial.

El sistema NO utilizará BLE.

Toda interacción con laptop se realizará mediante:

- USB-C
- monitor serial
- protocolos seriales
- comandos seriales
- telemetría serial
- debugging serial

---

---

# Política Oficial de Debugging

Todo módulo crítico debe poder depurarse mediante salida serial.

Esto incluye:

- sensores
- FSM
- event engine
- BLE
- scoring
- detección de eventos
- alertas
- botones
- pantalla

Formato sugerido:

```text
[TIME][MODULE][LEVEL] mensaje
```

Ejemplo:

```text
[10234][IMU][INFO] Impact detected
```

---

# Configuración Física Oficial del MVP

## Plataforma

Protoboard 830 puntos.

## MCU

ESP32 DevKit V1 USB-C.

## Interconexión

Dupont macho-macho.

## Alimentación

3.3V desde ESP32.

Todos los módulos operan a 3.3V.

---

# GPIO Mapping Oficial MVP

## Bus I2C

| Función | GPIO |
|---|---|
| SDA | GPIO21 |
| SCL | GPIO22 |

Dispositivos conectados:

- MAX30102
- MPU6050

---

## Pantalla GC9A01 SPI

| Señal | GPIO |
|---|---|
| MOSI | GPIO23 |
| CLK | GPIO18 |
| CS | GPIO5 |
| DC | GPIO2 |
| RST | GPIO4 |

Notas:

- BL conectado permanentemente a 3.3V
- sin control dinámico de backlight

---

## LED RGB

| Canal | GPIO |
|---|---|
| Rojo | GPIO14 |
| Verde | GPIO19 |
| Azul | GPIO27 |

Configuración:

- LED ánodo común
- resistencia 220Ω por canal
- PWM individual

---

## Botones

| Botón | GPIO |
|---|---|
| SW1 | GPIO25 |
| SW2 | GPIO26 |
| SW3 | GPIO33 |

Configuración:

```cpp
INPUT_PULLUP
```

Lógica:

- LOW = presionado
- HIGH = liberado

---

## Buzzer

| Señal | GPIO |
|---|---|
| SIGNAL | GPIO13 |

Configuración:

- PWM
- buzzer pasivo

---

# Sensores Oficiales MVP

## MAX30102

Funciones activas:

- BPM
- tendencia HRV básica
- señal PPG

Funciones NO prioritarias:

- precisión clínica SpO2

Restricciones:

- altamente sensible a movimiento
- lectura dependiente de contacto físico
- susceptible a ruido por cables largos

---

## MPU6050

Funciones activas:

- acelerómetro
- giroscopio
- detección de movimiento
- detección de impacto
- orientación básica

Frecuencia recomendada:

```text
50Hz–100Hz
```

---

# Política Oficial de Librerías

## Librerías base sugeridas

| Función | Librería |
|---|---|
| MAX30102 | heartRate + MAX30105 |
| MPU6050 | Adafruit MPU6050 o MPU6050_tockn |
| Pantalla | TFT_eSPI |
| BLE | NimBLE-Arduino |
| JSON | ArduinoJson |

---

# Política Oficial de Comunicación Serial

## Arquitectura de comunicación

SAM funcionará exclusivamente como dispositivo serial conectado por USB.

La laptop será:

- consola principal
- monitor de eventos
- herramienta de debugging
- interfaz de desarrollo
- sistema de validación

---

## Funciones oficiales del canal serial

El puerto serial será utilizado para:

- logs runtime
- telemetría sensores
- validación FSM
- monitoreo eventos
- calibración
- pruebas internas
- comandos debug
- exportación de datos

---

## Velocidad serial oficial

Configuración recomendada:

```cpp
115200
```

Puede aumentarse durante debugging intensivo.

---

## Política Oficial de Protocolos Seriales

El sistema debe permitir:

- comandos simples
- respuesta estructurada
- trazabilidad completa
- logs legibles
- exportación futura

Formato sugerido:

```text
[TIME][MODULE][LEVEL] DATA
```

---

## Política Oficial de Laptop Host

La laptop conectada se convierte oficialmente en:

- interfaz principal de desarrollo
- monitor del sistema
- herramienta de validación
- entorno de depuración

La pantalla local del wearable pasa a ser secundaria.

---

## Política Oficial de UI Local

La pantalla GC9A01 se utilizará únicamente para:

- estado general
- BPM
- alertas visuales
- mensajes básicos
- indicadores rápidos

La interfaz avanzada existirá en la laptop mediante serial monitor o herramientas futuras.

---

## Política Oficial de Eliminación BLE

BLE queda oficialmente removido del MVP actual.

Razones:

- reducción de complejidad
- menor consumo RAM
- menor complejidad firmware
- debugging más simple
- menor cantidad de fallos concurrentes
- aceleración del desarrollo
- prioridad absoluta en validación de sensores y FSM

---

## Consecuencias Técnicas de Remover BLE

### Ventajas

- más RAM disponible
- menor carga CPU
- menor complejidad RTOS
- menos errores asincrónicos
- debugging más estable
- arquitectura más simple

### Desventajas

- sin conexión móvil
- sin telemetría inalámbrica
- sin pruebas de app Flutter
- dependencia física del cable USB

---

## Política Oficial de Dependencia Física

SAM pasa oficialmente a ser un dispositivo tethered.

Esto significa:

- conexión física permanente a laptop
- alimentación permanente USB
- interacción principal por cable
- operación dependiente de host externo

---

# Política Oficial de Testing Hardware

## Orden oficial de validación

1. ESP32 standalone
2. Serial monitor
3. I2C scanner
4. MPU6050
5. MAX30102
6. Pantalla GC9A01
7. LED RGB
8. Botones
9. Buzzer
10. Integración total
11. comunicación serial avanzada
12. FSM
13. Event engine

---

# Política Oficial de Integración

Ningún módulo nuevo debe integrarse sin:

- validación independiente
- logs funcionales
- pruebas individuales
- aislamiento de fallos

---

# Política Oficial de Robustez

El firmware debe sobrevivir:

- desconexión sensor
- error I2C
- reinicio BLE
- pérdida temporal pantalla
- errores de lectura
- timeouts

El sistema debe degradarse parcialmente.

Nunca colapsar completamente.

---

# Política Oficial de Memoria

Evitar:

- uso excesivo de String
- buffers gigantes
- malloc continuo
- fragmentación heap

Preferir:

- estructuras fijas
- buffers circulares
- memoria estática

---

# Política Oficial de Timers

Quedan prohibidos:

```cpp
delay();
```

En lógica crítica.

Debe utilizarse:

- millis()
- timers RTOS
- tareas periódicas

---

# Política Oficial de FreeRTOS

Uso recomendado:

| Task | Prioridad |
|---|---|
| SensorTask | Alta |
| DetectionTask | Alta |
| CommunicationTask | Media |
| UITask | Baja |
| LoggingTask | Baja |

---

# Política Oficial de Modos del Sistema

## BOOT

Inicialización hardware.

---

## INIT

Validación sensores.

---

## NORMAL

Operación estándar.

---

## EVENT

Procesamiento evento crítico.

---

## ALERT_PENDING

Espera cancelación usuario.

---

## ALERT_SENT

Sistema transmite alerta mediante serial.

---

## RECOVERY

Verificación recuperación.

---

## ERROR

Sistema degradado.

---

# Política Oficial de Desarrollo Externo

Cualquier desarrollador externo debe asumir:

- hardware inestable de laboratorio
- protoboard como entorno principal
- prioridad firmware-first
- debugging intensivo serial
- evolución iterativa
- hardware no final

---

# Política Oficial de Migración Futura

La migración futura a PCB NO debe requerir:

- reescritura total firmware
- cambios drásticos arquitectura
- rediseño completo drivers

El sistema debe permanecer desacoplado del medio físico.

---

# Conclusión Técnica Oficial

El estado actual del proyecto SAM queda oficialmente redefinido como:

un sistema wearable MVP modular desarrollado sobre ESP32 DevKit V1 en protoboard, alimentado vía USB, utilizando PlatformIO como entorno principal y comunicación exclusivamente serial por USB-C.

La prioridad absoluta pasa a ser:

- validación funcional
- estabilidad del firmware
- arquitectura modular
- detección de eventos
- integración serial
- trazabilidad del sistema

La PCB personalizada deja temporalmente de ser el centro del desarrollo.

El firmware y la arquitectura lógica pasan a ser el núcleo principal del proyecto.

