#ifndef KEYPAD_HANDLER_H
#define KEYPAD_HANDLER_H

#include <Keypad.h>

class KeypadHandler {
public:
    KeypadHandler();
    char obtenerTecla();
private:
    static const byte FILAS = 4; // Número de filas
    static const byte COLUMNAS = 4; // Número de columnas

    char teclas[FILAS][COLUMNAS];
    byte pinesFilas[FILAS];
    byte pinesColumnas[COLUMNAS];

    Keypad keypad;
};

#endif
