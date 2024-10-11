
# Uso de la pantalla OLED con el ESP32 y el teclado matricial (I2C Genérico)

## Introducción

En esta configuración, utilizaremos la librería `U8g2` que es compatible con muchas placas y pantallas OLED, sin estar ligada a un vendor específico como esp8266 o ESP32. Esta librería es más general y funcionará en varias plataformas.

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

En este código, los pines del teclado matricial se han actualizado para evitar el uso de los pines 21 y 22 reservados para I2C.

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

byte pinesFilas[FILAS] = {33, 32, 25, 26};    // Pines de las filas conectadas al ESP32 (actualizados)
byte pinesColumnas[COLUMNAS] = {27, 14, 12, 13};  // Pines de las columnas conectadas al ESP32 (actualizados)

Keypad keypad = Keypad(makeKeymap(teclas), pinesFilas, pinesColumnas, FILAS, COLUMNAS);

// Configuración de la pantalla OLED (I2C genérico)
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C display(U8G2_R0, /* clock=*/ 22, /* data=*/ 21, /* reset=*/ U8X8_PIN_NONE); // Pines I2C originales

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

Este esquema funcionará en una amplia variedad de placas con I2C, incluyendo ESP32, Arduino y otras plataformas.

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
