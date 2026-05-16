DISTRIBUCIÓN GENERAL
Zona	Uso
Filas 1-15	ESP32
Filas 16-25	I2C sensores
Filas 26-40	Pantalla
Filas 41-50	LED + botones
Filas 51-60	buzzer
PASO 1 — Rails de alimentación
Conexiones rails

Haz estos puentes:

Rail rojo izquierdo ↔ Rail rojo derecho
Rail azul izquierdo ↔ Rail azul derecho

Hazlos:

arriba (fila 1)
o
abajo (fila 60)
PASO 2 — Posición ESP32

Coloca el ESP32:

Lado izquierdo
a1 → a15
Lado derecho
j1 → j15

Es decir:

ESP32 cruzando el canal central

con:

pines izquierdos en e
pines derechos en f

aproximadamente.

PASO 3 — Alimentación ESP32
ESP32 3V3

Conecta:

ESP32(3V3) → Rail rojo izquierdo
ESP32 GND

Conecta:

ESP32(GND) → Rail azul izquierdo
PASO 4 — Líneas I2C

Reserva:

Línea	Posición
SDA	fila 18
SCL	fila 19
GPIO21
GPIO21 → e18
GPIO22
GPIO22 → e19
PASO 5 — MPU6050
Posición recomendada

Filas:

22 → 27

lado izquierdo.

Alimentación
MPU6050	Protoboard
VCC	Rail rojo
GND	Rail azul
I2C
MPU6050	Línea
SDA	fila 18
SCL	fila 19
AD0
AD0 → Rail azul
PASO 6 — MAX30102
Posición

Filas:

30 → 35

lado izquierdo.

Alimentación
MAX30102	Protoboard
VIN	Rail rojo
GND	Rail azul
I2C
MAX30102	Línea
SDA	fila 18
SCL	fila 19
PASO 7 — Pantalla GC9A01

La pantalla NO conviene insertarla directamente.

Usa cables Dupont.

Zona recomendada

Filas:

20 → 35

lado derecho.

Conexiones pantalla
Pantalla	ESP32
VCC	Rail rojo
GND	Rail azul
SDA/MOSI	GPIO23
SCL/CLK	GPIO18
CS	GPIO5
DC	GPIO2
RST	GPIO4
PASO 8 — LED RGB
Posición

Filas:

45
Distribución recomendada
Pin LED	Posición
R	e45
común	f45
G	g45
B	h45
Conexiones
GPIO	Resistencia	LED
GPIO14	220Ω	rojo
GPIO12	220Ω	verde
GPIO27	220Ω	azul
Común
común → Rail rojo
PASO 9 — Botones
SW1

Filas:

48
SW2

Filas:

51
SW3

Filas:

54
Distribución

Cada botón:

cruzando canal central

Ejemplo SW1:

Lado	Posición
izquierdo	e48
derecho	f48
Conexiones
Botón	GPIO
SW1	GPIO25
SW2	GPIO26
SW3	GPIO33

Y el otro lado:

→ Rail azul
PASO 10 — Buzzer
Posición

Filas:

58-60
Conexiones
Buzzer	Conexión
+	GPIO13
-	Rail azul
RESUMEN FINAL FÍSICO
Rail rojo
3.3V común
Rail azul
GND común
Organización real lograda
Izquierda
I2C
sensores
Centro
ESP32
Derecha
pantalla SPI
Inferior
UI física
buzzer