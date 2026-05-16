---
name: debug_i2c
description: |
  Skill para diagnosticar y validar el bus I2C en el proyecto SAM. Contiene
  checklist, procedimiento obligatorio, pines actuales, ejemplos de logs y
  recomendaciones para evitar falsos positivos de fallo hardware.
scope: workspace
---

Objetivo

Diagnosticar problemas I2C en SAM y validar el bus antes de modificar drivers.

Problemas comunes

- Dirección incorrecta
- Cables SDA/SCL invertidos
- Falta de GND común
- Alimentación incorrecta o insuficiente
- Pull-ups duplicados o ausentes
- Ruido eléctrico
- Frecuencia I2C excesiva

Pines actuales (hardware SAM)

SDA -> GPIO21
SCL -> GPIO22

Procedimiento obligatorio

1. Verificar alimentación del sensor (VCC) y tensión correcta.
2. Comprobar GND común entre ESP32 y periféricos.
3. Ejecutar un I2C scanner y confirmar direcciones detectadas.
4. Validar que la dirección del sensor coincide con la esperada.
5. Reducir la frecuencia I2C a una valor seguro si es necesario.
6. Probar sensores individualmente desconectando otros dispositivos.

Herramienta obligatoria

Usar un I2C scanner antes de asumir fallo de software o driver.

Frecuencia segura (ejemplo)

```cpp
Wire.begin(21, 22, 100000);
```

Logs esperados

[I2C] device found: 0x68
[I2C] device found: 0x57

Direcciones esperadas (ejemplos)

- MPU6050: 0x68
- MAX30102: 0x57

NO permitido

- Asumir hardware defectuoso sin realizar las pruebas anteriores.
- Modificar drivers antes de validar el bus físico y las direcciones.
- Usar frecuencias extremas sin verificación (p. ej. >400kHz) en protoboard.

Recomendaciones prácticas

- Si no se detecta el dispositivo:
  - Verificar SDA/SCL con multímetro/física (cables sueltos).
  - Asegurar GND común.
  - Probar con solo el sensor conectado.
  - Añadir o revisar resistencias pull-up (4.7k–10k típicas).
- Si la detección es intermitente:
  - Reducir la velocidad I2C (p. ej. 100kHz).
  - Revisar longitud de cableado y ruido.

Prueba rápida de escaneo (ejemplo de salida esperada)

```
[I2C] scanning...
[I2C] device found: 0x68
[I2C] device found: 0x57
[I2C] done
```

Resultado esperado

- Bus I2C estable, detectable y consistente antes de cualquier cambio de driver.

Preguntas/ambigüedades

- ¿Quieres que incluya un script `i2c_scanner.ino` o un snippet de test en `/tests/`?
- ¿Preferimos documentar direcciones esperadas en `drivers/*/README.md` o en `config/`?

Ejemplos de prompts para invocar esta skill

- "Debug I2C: escanear bus y listar dispositivos encontrados"
- "Debug I2C: reducir frecuencia a 100k y reintentar detección"

FIN
