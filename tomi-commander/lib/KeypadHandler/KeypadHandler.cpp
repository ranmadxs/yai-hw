#include "KeypadHandler.h"

// Constructor que inicializa el teclado matricial
KeypadHandler::KeypadHandler()
    : teclas{{'1','2','3','A'},
             {'4','5','6','B'},
             {'7','8','9','C'},
             {'*','0','#','D'}},
      pinesFilas{32, 33, 25, 26},
      pinesColumnas{27, 14, 12, 13},
      keypad(makeKeymap(teclas), pinesFilas, pinesColumnas, FILAS, COLUMNAS) {}

char KeypadHandler::obtenerTecla() {
    return keypad.getKey();
}
