PARTE 1 — ARQUITECTURA GENERAL DEL SISTEMA SAM
Esta sección define la arquitectura global del sistema, estableciendo cómo se organizan los componentes físicos, el firmware, la comunicación y la aplicación móvil. El objetivo es construir una base técnica sólida que permita desarrollar un MVP funcional sin comprometer la evolución futura del sistema.
SAM no debe entenderse como un dispositivo aislado, sino como un sistema distribuido compuesto por tres capas principales: hardware embebido, lógica de procesamiento local (firmware) y capa de interacción externa (aplicación móvil).
________________________________________
Arquitectura conceptual del sistema
El sistema se organiza en tres dominios claramente delimitados:
1.	Dispositivo wearable (edge computing)
2.	Canal de comunicación (BLE)
3.	Aplicación móvil (control, visualización y alertas)
Cada uno de estos dominios tiene responsabilidades específicas y no debe invadir las funciones de los otros.
________________________________________
Principio de diseño clave
El sistema sigue una filosofía estricta:
La detección de eventos ocurre en el dispositivo, no en la nube.
Esto implica que:
•	el firmware debe ser autónomo
•	la latencia debe ser mínima
•	el sistema debe funcionar sin conexión a internet
La aplicación móvil actúa como extensión, no como dependencia crítica.
________________________________________
Dominio 1 — Dispositivo wearable
El wearable es el núcleo del sistema. Su responsabilidad es adquirir datos, procesarlos y tomar decisiones en tiempo real.
Componentes principales
•	MCU: ESP32 con BLE
•	Sensor PPG: MAX30102
•	Sensor de movimiento: MPU6050 (MVP)
•	Sensor de temperatura periférica
•	batería Li-Po
•	actuadores mínimos (LED, vibrador)
Responsabilidades
•	adquisición de señales fisiológicas y de movimiento
•	procesamiento en tiempo real
•	ejecución de la FSM
•	cálculo de features
•	evaluación de eventos
•	generación de alertas
•	comunicación BLE
Restricción crítica
El dispositivo debe operar bajo consumo energético estricto, lo que limita:
•	frecuencia de muestreo
•	complejidad de algoritmos
•	uso de comunicación
________________________________________
Dominio 2 — Canal de comunicación (BLE)
BLE no es un canal continuo de datos, sino un canal de eventos.
Principio
No transmitir todo. Transmitir solo lo relevante.
Tipos de comunicación
1.	Notificaciones de eventos
2.	Sincronización de estado
3.	Configuración
4.	Streaming temporal (debug o calibración)
Diseño del protocolo
El protocolo debe ser:
•	compacto
•	tolerante a fallos
•	orientado a eventos
El dispositivo nunca debe depender de confirmación externa para tomar decisiones críticas.
________________________________________
Dominio 3 — Aplicación móvil
La app es responsable de la interacción con el usuario y la gestión de alertas.
Funciones principales
•	visualización de estado del dispositivo
•	configuración del sistema
•	recepción de eventos
•	gestión de alertas
•	interfaz para el usuario y cuidadores
Restricción importante
La app no realiza detección de eventos. Solo interpreta y gestiona.
________________________________________
Flujo global del sistema
El flujo completo del sistema sigue una secuencia clara:
1.	el dispositivo adquiere datos de sensores
2.	el firmware procesa señales y extrae features
3.	el motor de estados evalúa condiciones
4.	si se detecta un evento:
o	se genera un confidence score
o	se valida mediante FSM
o	se inicia un proceso de alerta
5.	el evento se transmite vía BLE
6.	la app recibe y ejecuta el flujo de alerta
________________________________________
Separación de responsabilidades
Cada capa tiene límites estrictos:
Firmware
•	lógica crítica
•	detección
•	decisiones
BLE
•	transporte de información
App
•	experiencia de usuario
•	visualización
•	interacción
________________________________________
Arquitectura modular del firmware
El firmware no debe ser monolítico. Debe dividirse en módulos independientes:
•	adquisición de sensores
•	procesamiento de señales
•	extracción de features
•	motor de estados (FSM)
•	sistema de scoring
•	gestión de energía
•	comunicación BLE
Cada módulo debe poder evolucionar sin romper los demás.
________________________________________
Modelo de ejecución
El sistema debe operar bajo un modelo basado en tareas (idealmente con FreeRTOS en ESP32):
•	task de sensores
•	task de procesamiento
•	task de FSM
•	task de BLE
•	task de energía
Esto permite:
•	paralelismo controlado
•	priorización de tareas críticas
•	estabilidad en tiempo real
________________________________________
Gestión de tiempo
El sistema depende fuertemente del tiempo.
Debe garantizar:
•	timestamps consistentes
•	ventanas temporales correctas
•	sincronización entre sensores
Errores en tiempo implican:
•	falsos positivos
•	eventos mal detectados
________________________________________
Persistencia de datos
El dispositivo debe almacenar información mínima pero crítica:
•	baseline del usuario
•	parámetros adaptativos
•	configuración
Esto puede implementarse en:
•	NVS (Non-Volatile Storage del ESP32)
________________________________________
Escalabilidad del sistema
Aunque el MVP es simple, la arquitectura debe permitir:
•	agregar nuevos sensores
•	incorporar nuevos eventos
•	mejorar algoritmos
•	integrar análisis externo
Esto se logra mediante:
•	modularidad
•	abstracción de interfaces
•	separación clara de responsabilidades
________________________________________
Filosofía de diseño
SAM no busca ser un dispositivo médico, sino un sistema de detección temprana.
Por lo tanto:
•	se prioriza robustez sobre complejidad
•	se evita sobreingeniería
•	se diseñan algoritmos interpretables
________________________________________
Consideraciones críticas de ingeniería
1.	El sistema debe ser tolerante a ruido
2.	Debe operar en condiciones no ideales
3.	Debe minimizar falsos positivos
4.	Debe priorizar eventos críticos
5.	Debe ser energéticamente eficiente
________________________________________
Resultado esperado de esta arquitectura
Al implementar esta arquitectura correctamente se obtiene:
•	un sistema autónomo
•	robusto ante condiciones reales
•	eficiente en consumo
•	escalable
•	mantenible
________________________________________
PARTE 2 — ARQUITECTURA INTERNA DEL FIRMWARE Y PIPELINE DE PROCESAMIENTO
Esta sección describe cómo el firmware transforma señales crudas provenientes de sensores en decisiones concretas dentro del dispositivo. Aquí se define el flujo completo de datos, desde la adquisición hasta la generación de eventos, incluyendo filtrado, extracción de características, evaluación y control mediante el motor de estados.
El objetivo es que cualquier ingeniero pueda implementar el pipeline sin ambigüedades y entender claramente dónde ocurre cada operación crítica.
________________________________________
Visión general del pipeline
El firmware sigue un flujo determinista compuesto por etapas bien definidas:
1.	adquisición de señales
2.	preprocesamiento
3.	extracción de features
4.	evaluación de condiciones
5.	scoring
6.	decisión mediante FSM
7.	generación de evento
8.	comunicación
Cada etapa debe ser desacoplada y operar sobre estructuras de datos bien definidas.
________________________________________
Modelo de datos interno
El sistema debe trabajar con estructuras explícitas, evitando variables sueltas.
Estructura base de muestra
Cada lectura debe encapsularse en una estructura como:
•	timestamp
•	accel_x, accel_y, accel_z
•	gyro_x, gyro_y, gyro_z
•	hr (si disponible)
•	spo2 (opcional)
•	temperature
Buffer de datos
El sistema debe utilizar buffers circulares para:
•	mantener ventanas temporales
•	calcular features dinámicas
•	evitar pérdida de datos
El tamaño del buffer debe cubrir al menos:
•	2 a 5 segundos de información
________________________________________
Adquisición de sensores
La adquisición debe ser periódica y estable.
Frecuencias recomendadas
•	acelerómetro / giroscopio: 50–100 Hz
•	PPG: 25–50 Hz
Requisitos
•	lectura no bloqueante
•	manejo de errores de I2C
•	validación básica de datos
Errores en esta etapa contaminan todo el pipeline.
________________________________________
Preprocesamiento de señales
Las señales crudas contienen ruido y deben limpiarse antes de cualquier análisis.
Operaciones necesarias
•	filtrado low-pass para eliminar ruido de alta frecuencia
•	normalización de valores
•	eliminación de outliers extremos
Objetivo
Obtener señales estables sin perder eventos relevantes.
________________________________________
Cálculo de magnitudes
Para simplificar el análisis, se deben calcular magnitudes vectoriales.
Ejemplo
Magnitud de aceleración:
•	sqrt(x² + y² + z²)
Esto permite:
•	detectar impactos independientemente de la orientación
•	simplificar lógica de detección
________________________________________
Ventanas temporales
El sistema debe operar sobre ventanas deslizantes.
Tipos de ventanas
•	corta (100–300 ms): detección de impacto
•	media (1–2 s): análisis de movimiento
•	larga (5–10 s): inactividad
Cada ventana cumple una función específica en la detección.
________________________________________
Extracción de features
Las features son el núcleo del sistema de detección.
Features principales
•	magnitud de aceleración
•	varianza de aceleración
•	cambio brusco (delta)
•	energía de señal
•	duración de inactividad
•	orientación (opcional)
Requisito clave
Las features deben ser:
•	computacionalmente baratas
•	robustas al ruido
•	interpretables
________________________________________
Detección de condiciones primarias
Antes del scoring, el sistema debe identificar condiciones básicas.
Ejemplos
•	impacto detectado
•	cambio brusco de movimiento
•	periodo de inmovilidad
•	actividad elevada
Estas condiciones actúan como pre-filtros.
________________________________________
Sistema de scoring
El scoring combina múltiples features en un valor continuo.
Principio
No se toman decisiones binarias inmediatas.
Se calcula una probabilidad implícita.
Componentes del score
•	intensidad del impacto
•	variabilidad del movimiento
•	duración de inactividad
•	consistencia temporal
Cada componente tiene un peso definido.
________________________________________
Normalización del score
El score debe escalarse a un rango común, por ejemplo:
•	0.0 a 1.0
Esto permite:
•	comparar eventos
•	definir thresholds claros
•	facilitar ajustes dinámicos
________________________________________
Evaluación contra thresholds
El score se compara con thresholds adaptativos.
Tipos de thresholds
•	base (global)
•	dinámico (adaptado al usuario)
Resultado
•	score bajo → ignorar
•	score medio → observar
•	score alto → activar FSM
________________________________________
Integración con FSM
El FSM no procesa señales crudas, solo decisiones pre-evaluadas.
Entrada al FSM
•	tipo de evento
•	score
•	contexto temporal
Responsabilidad del FSM
•	validar el evento
•	evitar duplicados
•	gestionar tiempos
•	controlar escalamiento
________________________________________
Generación de eventos
Cuando el FSM confirma un evento:
Se crea una estructura que contiene:
•	tipo de evento
•	timestamp
•	score
•	duración
•	estado asociado
Este evento es el que se transmite vía BLE.
________________________________________
Pipeline de baja latencia
El sistema debe minimizar el tiempo entre:
detección → decisión → alerta
Requisito
La latencia total debe ser lo suficientemente baja para:
•	permitir cancelación por el usuario
•	reaccionar en tiempo útil
________________________________________
Manejo de errores
El pipeline debe ser resiliente.
Casos a manejar
•	pérdida de lecturas
•	valores inválidos
•	saturación de sensores
El sistema debe degradarse, no fallar.
________________________________________
Optimización computacional
Dado que el ESP32 tiene recursos limitados:
•	evitar operaciones costosas (ej. floats innecesarios)
•	reutilizar cálculos
•	limitar tamaño de buffers
________________________________________
Consistencia temporal
Todas las etapas deben basarse en el mismo reloj.
Errores aquí generan:
•	detecciones incorrectas
•	desalineación de eventos
________________________________________
Modo debug
El sistema debe incluir un modo especial para:
•	enviar datos crudos
•	visualizar features
•	analizar scoring
Esto es clave para calibración.
________________________________________
Resultado del pipeline
Si el pipeline está correctamente implementado:
•	los datos crudos se transforman en información útil
•	los eventos se detectan con contexto
•	el sistema mantiene estabilidad
________________________________________
PARTE 3 — MOTOR DE ESTADOS (FSM), DETECCIÓN DE EVENTOS Y LÓGICA DE DECISIÓN
Esta sección define el núcleo lógico del sistema SAM: el mecanismo que transforma señales procesadas y scores en decisiones estructuradas. Aquí no se trabaja con datos crudos, sino con interpretaciones ya procesadas del entorno físico del usuario.
El objetivo del FSM (Finite State Machine) es controlar el comportamiento del sistema en el tiempo, evitando decisiones impulsivas, reduciendo falsos positivos y garantizando consistencia en la detección de eventos críticos.
________________________________________
Principio fundamental del FSM
El sistema no debe reaccionar a eventos instantáneos, sino a patrones temporales.
Un evento como una caída no es un único dato, sino una secuencia:
•	impacto
•	desorientación
•	inmovilidad
El FSM existe para validar esa secuencia.
________________________________________
Estados principales del sistema
El sistema debe modelarse con un conjunto finito de estados claramente definidos.
Estados base
IDLE
Estado por defecto. El sistema monitorea continuamente sin condiciones relevantes.
ACTIVE_MONITORING
Se detecta actividad significativa. El sistema incrementa su atención.
POTENTIAL_EVENT
Se detecta una condición sospechosa (ej. impacto fuerte).
VALIDATING_EVENT
Se evalúa si el evento cumple condiciones temporales y físicas.
CONFIRMED_EVENT
El evento se considera válido.
ALERT_PENDING
Se ha confirmado el evento, pero se espera posible cancelación del usuario.
ALERT_SENT
La alerta ha sido enviada.
RECOVERY
El sistema observa si el usuario vuelve a un estado normal.
________________________________________
Transiciones entre estados
Las transiciones deben ser explícitas y controladas.
Flujo típico (ej. caída)
IDLE
→ (impacto detectado)
POTENTIAL_EVENT
→ (score alto + condiciones iniciales)
VALIDATING_EVENT
→ (inmovilidad confirmada)
CONFIRMED_EVENT
→ (timeout sin cancelación)
ALERT_PENDING
→ (sin interacción)
ALERT_SENT
→ (movimiento recuperado)
RECOVERY
→ IDLE
________________________________________
Eventos que activan transiciones
Las transiciones no ocurren arbitrariamente. Se basan en:
•	thresholds de score
•	condiciones temporales
•	patrones de movimiento
•	inputs del usuario
________________________________________
Condiciones temporales
El tiempo es un factor crítico.
Ejemplos
•	duración mínima de impacto
•	tiempo de inmovilidad (ej. 5–10 segundos)
•	ventana de cancelación (ej. 10–20 segundos)
El FSM debe usar timers internos para validar estas condiciones.
________________________________________
Sistema de validación progresiva
El FSM implementa una validación en etapas:
1.	detección inicial
2.	confirmación física
3.	validación temporal
4.	confirmación final
Esto reduce significativamente falsos positivos.
________________________________________
Integración con el scoring
El FSM no calcula el score, pero lo utiliza.
Uso del score
•	decidir entrada a POTENTIAL_EVENT
•	ajustar sensibilidad
•	priorizar eventos
Ejemplo
•	score < 0.4 → ignorar
•	0.4–0.7 → monitorear
•	0.7 → activar validación
________________________________________
Cancelación por el usuario
El sistema debe permitir intervención humana.
Mecanismos
•	botón físico
•	interacción en app
•	movimiento significativo
Regla
Si el usuario responde dentro del tiempo de gracia:
→ el evento se cancela
________________________________________
Prevención de duplicados
El FSM debe evitar múltiples alertas por el mismo evento.
Estrategia
•	bloquear nuevos eventos durante VALIDATING_EVENT
•	establecer cooldown tras ALERT_SENT
________________________________________
Manejo de múltiples eventos
El sistema debe manejar eventos consecutivos.
Reglas
•	no ignorar eventos reales durante RECOVERY
•	pero evitar sobrecarga de alertas
________________________________________
Estado de recuperación (RECOVERY)
Después de un evento, el sistema no vuelve inmediatamente a IDLE.
Objetivo
•	verificar estabilidad del usuario
•	evitar reactivaciones falsas
________________________________________
Eventos soportados (MVP)
El sistema debe centrarse en eventos de alto impacto:
Caídas
•	evento principal
Inmovilidad prolongada
•	posible pérdida de conciencia
Actividad anómala
•	comportamiento fuera del baseline
________________________________________
Extensibilidad del FSM
El diseño debe permitir agregar nuevos eventos sin reescribir todo.
Estrategia
•	FSM modular
•	submáquinas por tipo de evento
•	interfaces claras
________________________________________
Pseudocódigo conceptual del FSM
El comportamiento esperado puede representarse así:
switch(state):

  case IDLE:
    if score > threshold_event:
        state = POTENTIAL_EVENT

  case POTENTIAL_EVENT:
    if validate_initial_conditions():
        state = VALIDATING_EVENT
    else:
        state = IDLE

  case VALIDATING_EVENT:
    if immobility_detected():
        state = CONFIRMED_EVENT
    elif timeout_expired():
        state = IDLE

  case CONFIRMED_EVENT:
    start_alert_timer()
    state = ALERT_PENDING

  case ALERT_PENDING:
    if user_cancelled():
        state = IDLE
    elif timer_expired():
        send_alert()
        state = ALERT_SENT

  case ALERT_SENT:
    if recovery_detected():
        state = RECOVERY

  case RECOVERY:
    if stable():
        state = IDLE
________________________________________
Requisitos de implementación
•	el FSM debe ser determinista
•	no debe depender de delays bloqueantes
•	debe operar con timers no bloqueantes
•	debe ejecutarse en una task dedicada
________________________________________
Errores comunes a evitar
•	tomar decisiones en una sola muestra
•	no considerar contexto temporal
•	no manejar correctamente los estados intermedios
•	permitir transiciones ambiguas
________________________________________
Relación con la experiencia del usuario
El FSM impacta directamente en:
•	número de alertas falsas
•	tiempo de respuesta
•	confianza en el sistema
Un FSM mal diseñado vuelve inútil todo el sistema.
________________________________________
PARTE 4 — ADAPTIVE THRESHOLDS, PERFIL DEL USUARIO Y SISTEMA DE SCORING AVANZADO
Esta sección define cómo el sistema SAM evoluciona de un conjunto de reglas estáticas a un sistema adaptable que se ajusta al comportamiento real del usuario. Aquí se establece el mecanismo que permite reducir falsos positivos, mejorar la precisión y mantener estabilidad a largo plazo.
El objetivo es que el sistema no trate a todos los usuarios como iguales, sino que construya un perfil dinámico y utilice ese perfil para ajustar su sensibilidad.
________________________________________
Principio fundamental
Un sistema con thresholds fijos es inherentemente limitado.
El mismo movimiento puede ser normal para una persona y anómalo para otra.
Por lo tanto, el sistema debe aprender qué es “normal” para cada usuario y detectar desviaciones respecto a ese baseline.
________________________________________
Definición de baseline del usuario
El baseline representa el comportamiento típico del usuario en condiciones normales.
Variables a modelar
•	nivel promedio de actividad
•	variabilidad del movimiento
•	patrones de reposo
•	frecuencia cardíaca en reposo (si disponible)
•	rango típico de aceleraciones
Requisito
El baseline no es un valor único, sino un conjunto de estadísticas.
________________________________________
Inicialización del baseline
El sistema debe iniciar con valores por defecto conservadores.
Fase inicial
Durante las primeras horas o días:
•	se recolectan datos
•	no se aplican ajustes agresivos
•	se evita sobreajuste temprano
________________________________________
Actualización del baseline
El baseline debe actualizarse continuamente, pero de forma controlada.
Estrategia recomendada
Media móvil exponencial (EMA):
•	permite adaptación gradual
•	evita cambios bruscos
Regla crítica
No actualizar baseline durante eventos sospechosos.
________________________________________
Segmentación por contexto
El comportamiento del usuario cambia según el contexto.
Ejemplos
•	reposo
•	caminata
•	actividad alta
El sistema puede segmentar el baseline por estado para mejorar precisión.
________________________________________
Adaptive thresholds
Los thresholds deben ajustarse dinámicamente en función del baseline.
Ejemplo conceptual
Threshold de impacto:
•	usuario sedentario → threshold bajo
•	usuario activo → threshold más alto
Objetivo
Reducir falsos positivos sin perder sensibilidad.
________________________________________
Tipos de thresholds adaptativos
Threshold de impacto
Basado en la distribución de aceleración.
Threshold de inactividad
Basado en patrones de reposo del usuario.
Threshold de variabilidad
Basado en fluctuaciones normales de movimiento.
________________________________________
Límites de adaptación
La adaptación debe estar acotada.
Reglas
•	definir valores mínimos y máximos
•	evitar drift excesivo
•	mantener estabilidad
________________________________________
Sistema de scoring avanzado
El scoring debe evolucionar de un modelo simple a uno ponderado y contextual.
Estructura del score
El score final es una combinación de múltiples componentes:
•	score de impacto
•	score de variabilidad
•	score de inactividad
•	score contextual
________________________________________
Normalización de features
Cada feature debe transformarse a un rango común.
Esto permite:
•	combinar métricas heterogéneas
•	evitar dominancia de una sola variable
________________________________________
Ponderación de features
No todas las features tienen el mismo peso.
Ejemplo
•	impacto → peso alto
•	variabilidad → peso medio
•	contexto → peso ajustable
Los pesos deben poder ajustarse durante calibración.
________________________________________
Modelo de combinación
El score puede calcularse como:
suma ponderada de features normalizadas.
Esto mantiene:
•	interpretabilidad
•	bajo costo computacional
________________________________________
Confidence score
El resultado final es un valor continuo que representa la confianza del sistema en que un evento es real.
Interpretación
•	cercano a 0 → evento improbable
•	cercano a 1 → evento altamente probable
________________________________________
Uso del confidence score
El score no solo activa eventos, también permite:
•	priorizar alertas
•	ajustar tiempos de validación
•	diferenciar eventos críticos
________________________________________
Dependencia temporal del score
El score debe analizarse en el tiempo, no solo instantáneamente.
Ejemplo
Un pico aislado no es suficiente.
Una secuencia consistente sí.
________________________________________
Suavizado del score
Para evitar decisiones erráticas:
•	aplicar filtros temporales
•	evitar reacciones a ruido
________________________________________
Detección de anomalías
El sistema no solo detecta eventos definidos, también desviaciones del baseline.
Ejemplo
•	cambios en patrones de movimiento
•	reducción prolongada de actividad
________________________________________
Relación con el FSM
El FSM utiliza el score como entrada principal.
Interacción
•	score alto acelera transiciones
•	score medio prolonga validación
•	score bajo cancela eventos
________________________________________
Protección contra sobreajuste
El sistema no debe adaptarse demasiado rápido.
Estrategias
•	ventanas largas de aprendizaje
•	exclusión de eventos extremos
•	límites de cambio
________________________________________
Persistencia del perfil
El baseline y parámetros adaptativos deben almacenarse en memoria no volátil.
Esto permite:
•	continuidad entre sesiones
•	evitar recalibración constante
________________________________________
Modo recalibración
El sistema debe permitir reiniciar o ajustar el perfil.
Casos
•	cambio de usuario
•	comportamiento atípico prolongado
•	errores acumulados
________________________________________
Evolución futura
Este sistema puede escalar hacia:
•	modelos de machine learning ligeros
•	clustering de comportamiento
•	personalización avanzada
________________________________________
Impacto en el sistema completo
El adaptive system afecta directamente:
•	precisión
•	tasa de falsos positivos
•	experiencia del usuario
________________________________________
Errores comunes a evitar
•	adaptar thresholds demasiado rápido
•	usar features no normalizadas
•	depender de una sola variable
•	ignorar contexto temporal
________________________________________
PARTE 5 — COMUNICACIÓN BLE, PROTOCOLO, APLICACIÓN MÓVIL Y SISTEMA DE ALERTAS END-TO-END
Esta sección describe cómo el sistema SAM se conecta con el exterior y cómo los eventos detectados en el dispositivo se transforman en acciones concretas para el usuario o cuidadores. Aquí se define el contrato de comunicación, la arquitectura de la app y el flujo completo de alertas.
El objetivo es garantizar que un evento detectado en el firmware termine en una acción útil, confiable y oportuna en el mundo real.
________________________________________
Principio de diseño del sistema de comunicación
La comunicación no es continua ni masiva. Es selectiva y orientada a eventos.
El dispositivo decide. La app ejecuta.
Esto implica que el firmware mantiene la lógica crítica, mientras que la app se encarga de la interacción y visibilidad.
________________________________________
Modelo de comunicación BLE
BLE debe implementarse bajo un modelo basado en:
•	servicios
•	características
•	notificaciones
El dispositivo actúa como peripheral, la app como central.
________________________________________
Diseño del servicio principal
Se recomienda un único servicio principal para el MVP:
Service: SAM Service
Este servicio contiene múltiples características organizadas por responsabilidad.
________________________________________
Características principales
1. Event Characteristic
Responsable de enviar eventos detectados.
Contenido típico:
•	tipo de evento
•	timestamp
•	confidence score
•	duración
•	estado
Debe utilizar notificaciones (notify).
________________________________________
2. Status Characteristic
Describe el estado actual del dispositivo.
Ejemplos:
•	estado FSM
•	nivel de batería
•	estado de sensores
Se puede consultar bajo demanda o notificar cambios.
________________________________________
3. Config Characteristic
Permite configurar el dispositivo.
Parámetros:
•	thresholds base
•	sensibilidad
•	tiempos de validación
•	modo debug
Debe soportar escritura desde la app.
________________________________________
4. Control Characteristic
Permite enviar comandos desde la app.
Ejemplos:
•	cancelar evento
•	iniciar calibración
•	resetear baseline
________________________________________
5. Debug/Data Stream (opcional)
Para desarrollo y calibración.
Permite enviar:
•	datos crudos
•	features
•	scores
Debe desactivarse en producción para ahorrar energía.
________________________________________
Formato de datos
El protocolo debe ser compacto.
Recomendación
Utilizar estructuras binarias en lugar de JSON.
Ejemplo conceptual:
•	1 byte → tipo de evento
•	4 bytes → timestamp
•	1 byte → score (0–255)
•	1 byte → flags
Esto reduce:
•	consumo
•	latencia
•	uso de memoria
________________________________________
Manejo de conexión
El sistema debe ser tolerante a desconexiones.
Reglas
•	el dispositivo no depende de conexión activa
•	los eventos críticos pueden almacenarse temporalmente
•	la reconexión debe ser automática
________________________________________
Estrategia de transmisión
No todo se transmite.
Se transmite:
•	eventos confirmados
•	cambios importantes de estado
•	datos en modo debug
No se transmite:
•	datos crudos continuamente (en producción)
________________________________________
Arquitectura de la aplicación móvil
La app debe diseñarse con separación clara de capas:
•	capa de comunicación BLE
•	capa de lógica de negocio
•	capa de presentación
________________________________________
Módulos principales de la app
Gestor BLE
•	conexión
•	descubrimiento de servicios
•	lectura/escritura
•	manejo de reconexión
________________________________________
Gestor de eventos
•	recepción de eventos
•	interpretación del payload
•	almacenamiento local
________________________________________
Sistema de alertas
•	notificaciones locales
•	lógica de escalamiento
•	interacción con el usuario
________________________________________
Configuración
•	ajuste de parámetros
•	sincronización con dispositivo
________________________________________
Flujo completo de alerta
Este es el flujo crítico del sistema.
Secuencia
1.	el firmware detecta evento
2.	el FSM confirma evento
3.	se inicia ALERT_PENDING
4.	se envía evento vía BLE
5.	la app recibe evento
6.	se muestra alerta al usuario
7.	se inicia temporizador de respuesta
________________________________________
Interacción del usuario
El usuario puede:
•	confirmar que está bien
•	ignorar
•	no responder
________________________________________
Cancelación de alerta
Si el usuario responde:
→ la app envía comando de cancelación
→ el dispositivo vuelve a estado seguro
________________________________________
Escalamiento de alerta
Si no hay respuesta:
El sistema debe escalar:
1.	alerta local
2.	notificación persistente
3.	contacto de emergencia (futuro)
________________________________________
Diseño de UX para alertas
Debe ser:
•	clara
•	inmediata
•	difícil de ignorar
Pero sin ser intrusiva en falsos positivos.
________________________________________
Sincronización app–dispositivo
Debe mantenerse coherencia entre:
•	estado del FSM
•	estado mostrado en la app
________________________________________
Persistencia en la app
La app debe almacenar:
•	historial de eventos
•	configuraciones
•	estado del dispositivo
________________________________________
Modo offline
El sistema debe funcionar sin internet.
BLE es suficiente para el MVP.
________________________________________
Seguridad básica
Aunque es MVP, considerar:
•	emparejamiento BLE
•	validación de comandos
•	evitar accesos no autorizados
________________________________________
Consumo energético en BLE
BLE debe usarse eficientemente:
•	intervalos de advertising controlados
•	conexión solo cuando sea necesario
•	notificaciones cortas
________________________________________
Errores comunes a evitar
•	transmitir datos innecesarios
•	depender de conexión constante
•	usar payloads grandes
•	no manejar reconexión

