
# Conexión del ESP32 con un teclado matricial 4x4

## Introducción

Este archivo describe cómo conectar un teclado matricial 4x4 a un ESP32, el pinout necesario para la conexión y un código de ejemplo para PlatformIO. El código envía un mensaje por serial cada vez que se presiona un botón en el teclado.

## Pinout entre el Keypad 4x4 y el ESP32

El teclado matricial 4x4 tiene 8 pines que corresponden a las **filas** y **columnas** de la matriz de botones. Estos pines deben conectarse a los pines GPIO del ESP32.

### Conexión de pines

| Pin del Keypad | Función  | Pin en ESP32 |
|----------------|----------|--------------|
| **R1**         | Fila 1   | GPIO 23      |
| **R2**         | Fila 2   | GPIO 22      |
| **R3**         | Fila 3   | GPIO 21      |
| **R4**         | Fila 4   | GPIO 19      |
| **C1**         | Columna 1| GPIO 18      |
| **C2**         | Columna 2| GPIO 5       |
| **C3**         | Columna 3| GPIO 17      |
| **C4**         | Columna 4| GPIO 16      |

## Pinout estándar de un teclado matricial 4x4:

En los teclados matriciales 4x4 como el tuyo, los 8 pines (en la parte inferior) suelen estar organizados de la siguiente manera:

1. **R1 (Fila 1)**: Primera fila (botones 1, 2, 3, A)
2. **R2 (Fila 2)**: Segunda fila (botones 4, 5, 6, B)
3. **R3 (Fila 3)**: Tercera fila (botones 7, 8, 9, C)
4. **R4 (Fila 4)**: Cuarta fila (botones *, 0, #, D)
5. **C1 (Columna 1)**: Primera columna (botones 1, 4, 7, *)
6. **C2 (Columna 2)**: Segunda columna (botones 2, 5, 8, 0)
7. **C3 (Columna 3)**: Tercera columna (botones 3, 6, 9, #)
8. **C4 (Columna 4)**: Cuarta columna (botones A, B, C, D)

### Orden de los pines en la tira del keypad (parte inferior):

```
        [Conector Keypad]
          | | | | | | | |
          R1 R2 R3 R4 C1 C2 C3 C4
```

### Descripción de los pines:

- **R1 - R4**: Corresponden a las **filas** del teclado (de arriba hacia abajo).
  - R1: Botones 1, 2, 3, A
  - R2: Botones 4, 5, 6, B
  - R3: Botones 7, 8, 9, C
  - R4: Botones *, 0, #, D

- **C1 - C4**: Corresponden a las **columnas** (de izquierda a derecha).
  - C1: Botones 1, 4, 7, *
  - C2: Botones 2, 5, 8, 0
  - C3: Botones 3, 6, 9, #
  - C4: Botones A, B, C, D

### Conexión entre pines y ESP32:

Ahora que sabes qué pin corresponde a cada fila o columna en el conector del keypad, puedes conectarlos a los pines GPIO del ESP32 según el siguiente esquema:

- R1 → GPIO 23
- R2 → GPIO 22
- R3 → GPIO 21
- R4 → GPIO 19
- C1 → GPIO 18
- C2 → GPIO 5
- C3 → GPIO 17
- C4 → GPIO 16

Si quieres verificar esto físicamente, puedes usar un multímetro en modo de continuidad y presionar los botones correspondientes para confirmar que los pines se activan según lo esperado.

## Código para PlatformIO

A continuación, un código de ejemplo para el ESP32 utilizando PlatformIO que permite detectar cuando se presiona un botón del teclado y envía un mensaje por serial indicando cuál fue el botón presionado.

### Instalación de Librerías

En tu archivo `platformio.ini`, asegúrate de incluir la librería `Keypad`:

```ini
[env:esp32]
platform = espressif32
board = esp32dev
framework = arduino

lib_deps = 
    keypad
```

### Código para `main.cpp`

```cpp
#include <Arduino.h>
#include <Keypad.h>

// Configuración del teclado matricial
const byte FILAS = 4; // Número de filas
const byte COLUMNAS = 4; // Número de columnas

char teclas[FILAS][COLUMNAS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte pinesFilas[FILAS] = {23, 22, 21, 19};    // Pines de las filas conectadas al ESP32
byte pinesColumnas[COLUMNAS] = {18, 5, 17, 16};  // Pines de las columnas conectadas al ESP32

Keypad keypad = Keypad(makeKeymap(teclas), pinesFilas, pinesColumnas, FILAS, COLUMNAS);

void setup(){
  Serial.begin(115200);
}

void loop(){
  char tecla = keypad.getKey();

  if (tecla){
    Serial.print("Apretaste el botón ");
    Serial.println(tecla);
  }
}
```

## Explicación

- El código utiliza la librería `Keypad` para manejar el teclado matricial.
- En la función `setup()` se inicializa el puerto serial a 115200 baudios.
- En el `loop()`, se verifica si hay una tecla presionada con `keypad.getKey()`. Si se presiona una tecla, se envía un mensaje por serial indicando cuál fue.
