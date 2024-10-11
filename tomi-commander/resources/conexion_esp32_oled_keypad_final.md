
# Conexión del ESP32 con un teclado matricial 4x4

## Introducción

Este archivo describe cómo conectar un teclado matricial 4x4 a un ESP32, el pinout necesario para la conexión y un código de ejemplo para PlatformIO. El código envía un mensaje por serial cada vez que se presiona un botón en el teclado.

## Pinout entre el Keypad 4x4 y el ESP32

El teclado matricial 4x4 tiene 8 pines que corresponden a las **filas** y **columnas** de la matriz de botones. Estos pines deben conectarse a los pines GPIO del ESP32.

### Conexión de pines

| Pin del Keypad | Función  | Pin en ESP32 |
|----------------|----------|--------------|
| **R1**         | Fila 1   | GPIO 33      |
| **R2**         | Fila 2   | GPIO 32      |
| **R3**         | Fila 3   | GPIO 25      |
| **R4**         | Fila 4   | GPIO 26      |
| **C1**         | Columna 1| GPIO 27      |
| **C2**         | Columna 2| GPIO 14      |
| **C3**         | Columna 3| GPIO 12      |
| **C4**         | Columna 4| GPIO 13      |

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

- R1 → GPIO 33
- R2 → GPIO 32
- R3 → GPIO 25
- R4 → GPIO 26
- C1 → GPIO 27
- C2 → GPIO 14
- C3 → GPIO 12
- C4 → GPIO 13

Si quieres verificar esto físicamente, puedes usar un multímetro en modo de continuidad y presionar los botones correspondientes para confirmar que los pines se activan según lo esperado.



# Uso de la pantalla OLED con el ESP32 y el teclado matricial (I2C Genérico)

## Introducción

Este archivo contiene la configuración y el código necesario para conectar un teclado matricial y una pantalla OLED al ESP32. Utilizaremos la librería `U8g2`, que es compatible con muchas pantallas OLED y no está ligada a un vendor específico.

## Estructura de directorios y nombres de archivos

A continuación, se muestra la estructura de directorios que debes tener para el proyecto en PlatformIO:

```
/your_project_directory
│
├── /include                 # Archivos de cabecera opcionales
├── /lib                     # Librerías externas si se requieren
├── /src
│   └── main.cpp             # Código fuente principal
├── /test                    # Directorio para pruebas
├── platformio.ini           # Archivo de configuración de PlatformIO
```

### Contenido del archivo `platformio.ini`:

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino

lib_deps =
    keypad
    olikraus/U8g2 @ ^2.34.15
```

## Código fuente en `src/main.cpp`:

El siguiente código permite gestionar tanto el teclado matricial como la pantalla OLED:

```cpp
#include <Arduino.h>
#include <Keypad.h>
#include <Wire.h>
#include <U8g2lib.h>

// Configuración del teclado matricial
const byte FILAS = 4; // Número de filas
const byte COLUMNAS = 4; // Número de columnas

char teclas[FILAS][COLUMNAS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

// Pines de las filas y columnas conectadas al ESP32 (sin usar GPIO 21 y 22)
byte pinesFilas[FILAS] = {33, 32, 25, 26};    // Pines de las filas actualizados
byte pinesColumnas[COLUMNAS] = {27, 14, 12, 13};  // Pines de las columnas actualizados

Keypad keypad = Keypad(makeKeymap(teclas), pinesFilas, pinesColumnas, FILAS, COLUMNAS);

// Configuración de la pantalla OLED (I2C genérico)
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C display(U8G2_R0, /* clock=*/ 22, /* data=*/ 21, /* reset=*/ U8X8_PIN_NONE); // Pines I2C para OLED

void setup() {
  Serial.begin(115200);

  // Inicializar la pantalla OLED
  display.begin();
  display.clearBuffer();
  display.setFont(u8g2_font_ncenB08_tr);
  display.drawStr(0, 10, "Teclado listo");
  display.sendBuffer(); // Enviar al display
}

void loop() {
  char tecla = keypad.getKey();

  if (tecla) {
    // Mostrar en el puerto serial
    Serial.print("Apretaste el botón ");
    Serial.println(tecla);

    // Mostrar en la pantalla OLED
    display.clearBuffer();
    display.drawStr(0, 10, "Apretaste el boton ");
    display.setCursor(0, 20);
    display.print(tecla);
    display.sendBuffer(); // Enviar al display
  }
}
```

## Pinout de la pantalla OLED con el ESP32

En la imagen proporcionada, los pines de la pantalla OLED están etiquetados como:

- **GND**: Tierra.
- **VCC**: Alimentación de 3.3V.
- **SCL**: Línea de reloj del bus I2C.
- **SDA**: Línea de datos del bus I2C.

### Esquema de Pines para la Pantalla OLED (I2C)

| Pin de la pantalla OLED | Función  | Pin en ESP32  |
|-------------------------|----------|---------------|
| **GND**                 | Tierra   | GND           |
| **VCC**                 | Alimentación (3.3V) | 3.3V         |
| **SCL**                 | Reloj I2C | GPIO 22       |
| **SDA**                 | Datos I2C | GPIO 21       |

### Esquema gráfico:

```
Pantalla OLED   ESP32
-------------------------
GND  ---------- GND
VCC  ---------- 3.3V
SCL  ---------- GPIO 22
SDA  ---------- GPIO 21
```

## Compilación y carga del código en PlatformIO

### Pasos para compilar y cargar el código en ESP32 utilizando PlatformIO:

1. **Abrir el proyecto en PlatformIO (VS Code)**: Asegúrate de que tienes PlatformIO instalado en Visual Studio Code. Abre el directorio del proyecto.

2. **Seleccionar el entorno de compilación**: Asegúrate de que el archivo `platformio.ini` esté correctamente configurado para ESP32, como se muestra arriba.

3. **Compilar el código**: Para compilar el código, usa el ícono de la "marca de verificación" en la barra inferior de PlatformIO o ejecuta el comando siguiente en la terminal de PlatformIO:

    ```bash
    pio run
    ```

4. **Subir el código al ESP32**: Conecta tu ESP32 a tu computadora a través del cable USB y asegúrate de seleccionar el puerto correcto. Luego, sube el código al ESP32 usando el ícono de la "flecha hacia la derecha" o ejecuta el siguiente comando:

    ```bash
    pio run --target upload
    ```

5. **Monitorear el puerto serial**: Puedes observar los mensajes en el puerto serial desde la consola de PlatformIO con el siguiente comando:

    ```bash
    pio device monitor
    ```

Esto mostrará las salidas de depuración en el puerto serial, como los mensajes que indican qué botón se ha presionado en el teclado.

## Notas adicionales:

- Si tienes problemas con la carga del código, asegúrate de que el ESP32 esté bien conectado y que el driver del puerto serial esté instalado.
- Si la pantalla no muestra nada, verifica las conexiones I2C (SCL y SDA) y la dirección I2C correcta (0x3C es común para pantallas SSD1306).
