---
name: create_platformio_module
description: |
  Skill para crear módulos organizados y compatibles con la arquitectura SAM y
  PlatformIO. Provee plantilla, reglas obligatorias, convenciones de inclusión
  y recomendaciones sobre `main.cpp` y la ubicación de configuraciones.
scope: workspace
---

Objetivo

Crear módulos reutilizables, limpios y desacoplados que encajen en la estructura
del firmware SAM y funcionen correctamente bajo PlatformIO.

Estructura obligatoria

Cada módulo debe tener una carpeta propia con al menos:

```
/module_name/
    module_name.h
    module_name.cpp
```

Reglas y convenciones

- Separar interfaz (`.h`) e implementación (`.cpp`).
- Evitar lógica gigante en un solo archivo; dividir responsabilidades.
- No poner lógica de módulo en `main.cpp`.

`main.cpp` — responsabilidades limitadas

`main.cpp` solo debe:
- Inicializar el sistema (hardware mínimo, logging, config).
- Registrar y crear tasks FreeRTOS.
- Inicializar módulos (llamadas a `init()` públicas).
- Iniciar scheduler / loop principal.

Includes

- Preferir includes mínimos en headers; usar forward declarations cuando sea posible.
- Incluir solo lo necesario en `module_name.h` para minimizar acoplamiento.
- Evitar includes recíprocos; si aparecen, refactorizar interfaces.

Configuración y constantes

- Todas las constantes configurables deben vivir en `config/` como headers
  dedicados (por ejemplo `config/module_config.h`).
- Evitar macros distribuidas dentro de múltiples módulos; centralizar en `config/`.

Buenas prácticas de implementación

- Proveer funciones públicas claras: `bool init()`, `void shutdown()`, getters básicos.
- Evitar lógica de aplicación (FSM, detección) dentro del módulo; exponer datos.
- Documentar pines, I2C/SPI addresses y dependencias en `drivers` o en `README.md` dentro
  del directorio del módulo.
- Manejar errores internamente y exponer estados (p. ej. `isInitialized()`).

Tests y ejemplos

- Incluir un `README.md` con instrucciones de verificación manual y pines usados.
- Opcional: añadir un fichero de test en `/tests/` que inicialice el módulo y muestre
  lecturas por `Serial`.

Resultado esperado

- Módulos reutilizables, limpios, desacoplados y mantenibles que puedan integrarse
  fácilmente en `main.cpp` sin añadir lógica de negocio al arranque.

Checklist antes de merge

- [ ] Interfaz y implementación separadas
- [ ] `init()` documentado y no bloqueante
- [ ] Configuraciones en `config/` o claramente documentadas
- [ ] README.md con pines y dependencias
- [ ] No incluye lógica de UI/FSM

Preguntas/ambigüedades para el autor

- ¿Deseas que el skill también genere un skeleton de módulo (`.h` + `.cpp`) automáticamente?
- ¿Preferimos centralizar todas las macros/addresses en `config/` o documentarlas en cada driver?
- ¿Quieres que incluya un ejemplo de `platformio.ini` target para el módulo (build flags)?

Ejemplos de prompts para invocar esta skill

- "Create module: scaffold `ble_interface` module skeleton"
- "Create module: scaffold `motion_analyzer` with config header and README"

FIN
