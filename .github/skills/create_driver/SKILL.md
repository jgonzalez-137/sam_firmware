---
name: create_driver
description: |
  Skill para crear drivers modulares, mantenibles y desacoplados para hardware
  usado en el proyecto SAM (I2C, SPI, ADC, PWM y periféricos simples). Provee
  plantilla, convenciones, checklist y ejemplos mínimos de interfaz y manejo de
  errores.
scope: workspace
---

Objetivo

Crear drivers modulares, mantenibles y desacoplados que inicialicen hardware,
lean datos, validen comunicación y expongan lecturas limpias sin contener
lógica de negocio ni FSM.

Ámbito de aplicación

- Sensores I2C (ej. MPU6050, MAX30102)
- Periféricos SPI (ej. pantallas)
- ADC / entradas analógicas
- Señales PWM (buzzer, LED)

Arquitectura esperada y estructura de archivos

Cada driver debe vivir en su propio directorio:

```
/drivers/<driver_name>/
    <driver_name>.h
    <driver_name>.cpp
    README.md    # pines, direcciones, notas de validación
```

Reglas obligatorias

NO permitido dentro del driver:
- Lógica de negocio o detección de eventos
- FSMs o manejo de estados de aplicación
- UI / renderizado de pantalla
- Delays largos o bloqueos (no usar `delay()`)
- Variables globales innecesarias

Permitido y esperado:
- Inicializar hardware y configurar registros
- Leer datos y validar comunicación
- Convertir datos crudos a unidades básicas
- Manejar y reportar errores hardware

Interfaz mínima recomendada

```cpp
// tipos compactos para lecturas
struct ImuSample {
    float ax;
    float ay;
    float az;
    float gx;
    float gy;
    float gz;
    bool valid;
};

class Mpu6050Driver {
public:
    // inicializa hardware, valida comunicación; devuelve true si ok
    bool init();

    // lectura no bloqueante; actualiza lectura interna y return true si hubo new data
    bool update();

    // estado de conexión/health
    bool isConnected() const;

    // obtener la última muestra (copia ligera)
    ImuSample read();
};
```

Manejo de errores

- Validar cada transferencia I2C/SPI/ADC (timeouts, ACKs, valores NaN).
- `init()` debe devolver `false` si la inicialización falla; nunca asumir éxito.
- `update()` debe ser rápida y tolerante a fallos (reintentos internos limitados).
- Exponer estados y errores mediante métodos o flags, no `Serial`-only.

Formato de logging

- Prefix recomendado: `[DRIVER:<name>]` y/o `[I2C]` / `[SPI]` según corresponda.
- Mensajes mínimos: init success/fail, desconexión detectada, lectura inválida.
- Ejemplo: `Serial.println("[MPU6050] init success");`

Estructuras y memoria

- Evitar uso innecesario de memoria dinámica.
- Priorizar structs compactos; evitar arrays grandes en stack sin control.
- Evitar uso excesivo de `float` si no es necesario; documentar cualquier trade-off.

Inicialización y update

- `init()` debe:
  - configurar periférico
  - validar comunicación (leer WHO_AM_I, registros de identificación, etc.)
  - dejar el driver en estado determinista

- `update()` debe:
  - ser no bloqueante y rápida
  - leer/parsear datos y almacenar en una estructura interna
  - devolver si hay nueva data para consumo

Optimización y buenas prácticas

- Priorizar estabilidad y claridad sobre micro-optimización prematura.
- Documentar pines, dirección I2C y requisitos de energía en `README.md`.
- Mantener bajo uso de RAM y evitar buffers innecesarios.

Checklist antes de merge

- [ ] `init()` valida comunicación y devuelve `false` en fallo
- [ ] `update()` es no bloqueante y documentado
- [ ] `isConnected()` refleja estado real del bus
- [ ] Logs mínimos incluidos y consistentes
- [ ] Archivos colocados en `/drivers/<name>/` con `README.md`
- [ ] No hay lógica de detección dentro del driver

Decisiones abiertas / preguntas para el autor

- ¿Preferencia por usar `namespace sam` en drivers o clases en scope global?
- ¿API pública como `class` o estilo C con `struct` + funciones? (recomiendo `class`)
- ¿Deseas centralizar direcciones/macros en `config/i2c_addresses.h` o dejarlas
  en el `README` de cada driver?

Ejemplos de prompts para usar esta skill

- "Create driver: scaffold I2C driver for MPU6050 with init/update/isConnected"
- "Create driver: implement MAX30102 read path and basic validation"

FIN
