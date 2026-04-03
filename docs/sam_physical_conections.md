# 🔌 Documentación de Conexiones – Hardware MVP Wearable

Esta documentación define **todas las conexiones físicas necesarias** para el prototipo completo, listo para pruebas de rendimiento y consumo.

---

# Definición de Nodos

Para evitar ambigüedades en conexiones compartidas:

| Nodo     | Descripción |
|----------|------------|
| GND_SYS  | Tierra común del sistema |
| VCC_3V3  | Salida 3.3V del ESP32 |
| VCC_BAT  | Salida del TP4056 (OUT+) |
| I2C_SDA  | Bus SDA compartido |
| I2C_SCL  | Bus SCL compartido |

---

# 1. Sistema de Energía

## Batería → TP4056

| Componente | Pin | Conectar a |
|-----------|-----|------------|
| Batería | + | TP4056 B+ |
| Batería | - | TP4056 B- |

---

## TP4056 → Sistema

| TP4056 | Conectar a |
|--------|-----------|
| OUT+ | VCC_BAT |
| OUT- | GND_SYS |

---

## ESP32 Alimentación

| ESP32 | Conectar a |
|------|------------|
| VIN | VCC_BAT |
| GND (GND_1) | GND_SYS |
| 3V3 | VCC_3V3 |

---

# 2. Bus I²C (Compartido)

## Nodo I2C_SDA

| Pin | Conectado a |
|-----|-------------|
| GPIO21 (ESP32) | I2C_SDA |
| MAX30102 SDA | I2C_SDA |
| MPU6050 SDA | I2C_SDA |

---

## Nodo I2C_SCL

| Pin | Conectado a |
|-----|-------------|
| GPIO22 (ESP32) | I2C_SCL |
| MAX30102 SCL | I2C_SCL |
| MPU6050 SCL | I2C_SCL |

---

# 3. Sensor MAX30102

| MAX30102 | Conectar a |
|----------|------------|
| VIN | VCC_3V3 |
| GND (GND_2) | GND_SYS |
| GND (GND_3) | GND_SYS |
| SDA | I2C_SDA |
| SCL | I2C_SCL |
| INT | GPIO27 |

> Nota: RD e IRD no se conectan.

---

# 4. Sensor MPU6050 (GY-521)

| MPU6050 | Conectar a |
|----------|------------|
| VCC | VCC_3V3 |
| GND (GND_4) | GND_SYS |
| SDA | I2C_SDA |
| SCL | I2C_SCL |
| INT | GPIO26 |
| ADO | GND_SYS |

### Pines no utilizados

| Pin | Estado |
|-----|--------|
| XDA | Sin conectar |
| XCL | Sin conectar |

---

# 5. Pantalla GC9A01 (SPI)

## Bus SPI

| Señal | ESP32 | Pantalla |
|------|--------|---------|
| SCLK | GPIO18 | SCL |
| MOSI | GPIO23 | SDA |
| CS | GPIO5 | CS |
| DC | GPIO16 | DC |
| RST | GPIO17 | RST |

---

## Alimentación

| Pantalla | Conectar a |
|----------|------------|
| VCC | VCC_3V3 |
| GND (GND_5) | GND_SYS |
| BL | VCC_3V3 |

---

# 6. Zumbador

| Zumbador | Conectar a |
|----------|------------|
| + | GPIO25 |
| - | GND_SYS |

---

# 7. Micrófono

| Micrófono | Estado |
|----------|--------|
| + | No conectar |
| - | No conectar |

> Requiere amplificación externa (no usable en esta fase).

---

# 8. Tierra Común

Todos los GND deben estar conectados:


GND_SYS =
ESP32 GND_1
TP4056 OUT-
MAX30102 GND_2, GND_3
MPU6050 GND_4
Pantalla GND_5
Zumbador -

---

# 9. Distribución de Energía

Batería → TP4056 → VCC_BAT → ESP32 VIN

ESP32 3V3 → VCC_3V3 →
├── MAX30102
├── MPU6050
└── Pantalla

---

# 10. Uso de Pines ESP32

| Pin | Uso |
|-----|-----|
| VIN | Entrada batería |
| 3V3 | Alimentación sensores |
| GND | Tierra |
| GPIO21 | I2C SDA |
| GPIO22 | I2C SCL |
| GPIO27 | INT MAX30102 |
| GPIO26 | INT MPU6050 |
| GPIO18 | SPI SCLK |
| GPIO23 | SPI MOSI |
| GPIO5 | CS pantalla |
| GPIO16 | DC pantalla |
| GPIO17 | RST pantalla |
| GPIO25 | Zumbador |

---

# Estado del Sistema

Con esta configuración:

- Sistema autónomo funcional  
- Sensores operativos con interrupciones  
- Pantalla activa  
- Actuador básico integrado  
- Listo para pruebas de consumo  

---

# Nota Técnica

Esta configuración representa:

**Consumo máximo del sistema (peor escenario)**

No representa:
- Consumo optimizado
- Autonomía final del producto

---
