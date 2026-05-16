---
name: create_fsm
description: |
  Skill para diseñar e implementar máquinas de estados finitas deterministas (FSM)
  para el firmware SAM. Provee plantilla, reglas obligatorias, convenciones de
  logging y checklist de validación para asegurar determinismo y claridad.
scope: workspace
---

Objetivo

Crear máquinas de estados deterministas para SAM que centralicen el manejo de
eventos importantes y mantengan la lógica de control separada de sensores y UI.

Filosofía

- Todo evento importante debe pasar por la FSM.
- Evitar decisiones dispersas o lógica implícita.
- No usar `delay()` ni bloqueos; preferir timers no bloqueantes.

Reglas obligatorias

- La FSM debe ser determinista.
- Usar `enum class` para los estados.
- Transiciones explícitas y validadas.
- Evitar delays bloqueantes y bucles infinitos.

Estructura recomendada (esqueleto)

```cpp
enum class SystemState {
    IDLE,
    MONITORING,
    EVENT,
    ALERT
};

class SystemFsm {
public:
    void update();
    void transitionTo(SystemState newState);
    SystemState getCurrentState() const;
};
```

Reglas de transición

- Cada transición debe ser invocada mediante `transitionTo(...)`.
- Las transiciones deben validar condiciones pre/post y registrar un log.
- Registrar siempre: `[FSM] <FROM> -> <TO>`.

Logging obligatorio

- Ejemplo de log: `[FSM] IDLE -> EVENT`.
- Incluir razón breve (p. ej. `I2C timeout`, `threshold exceeded`).

Timers y temporización

- Usar `millis()` y timers no bloqueantes.
- Nunca usar `delay()` dentro de la FSM.

Restricciones de responsabilidades

- La FSM NO debe leer sensores directamente.
- La FSM NO debe controlar UI directamente.
- Consumir únicamente eventos ya procesados por `services/` o `drivers/`.

NO permitido

- Estados ambiguos o superpuestos.
- Lógica duplicada entre transiciones.
- Múltiples estados activos a la vez.
- Bucles `while(true)` bloqueantes o `delay()` prolongados.

Resultado esperado

- FSM clara, extensible, estable y fácil de depurar.

Checklist antes de merge

- [ ] `enum class` usado para estados
- [ ] Todas las transiciones pasan por `transitionTo()` y registran logs
- [ ] `update()` es no bloqueante
- [ ] La FSM consume eventos, no sensores directos
- [ ] Casos borde validados y fallback seguros

Ambigüedades y preguntas

- ¿Prefieres que las FSMs estén en `core/` o en `services/`?
- ¿Deseas un helper de logs uniforme para `[FSM]` o usar `Serial.printf` directo?
- ¿Soportarás subestados (hierarchical FSM) o solo flat enums inicialmente?

Ejemplos de prompts para invocar esta skill

- "Create FSM: scaffold SystemFsm with IDLE, MONITORING, EVENT, ALERT"
- "Create FSM: add transition guard for I2C timeout -> fallback state"

Siguientes personalizaciones sugeridas

- Plantilla de FSM (skeleton generator) que cree archivos en `/core/fsm/`.
- `.instructions.md` con convenciones de logs y naming de estados.

FIN
