#include "OledDisplay.h"
#include <Wire.h>

// Constructor que inicializa la pantalla con los pines I2C
OledDisplay::OledDisplay() 
    : display(U8G2_R0, /* clock=*/ 22, /* data=*/ 21, /* reset=*/ U8X8_PIN_NONE) {}

void OledDisplay::iniciarPantalla() {
    display.begin();
    display.clearBuffer();
    display.setFont(u8g2_font_ncenB08_tr);
    display.drawStr(0, 10, "Teclado listo");
    display.sendBuffer(); // Enviar al display
}

void OledDisplay::mostrarMensaje(const char* mensaje) {
    display.clearBuffer();
    display.drawStr(0, 10, mensaje);
    display.sendBuffer(); // Enviar al display
}

void OledDisplay::mostrarTecla(char tecla) {
    display.clearBuffer();
    display.drawStr(0, 10, "Apretaste el boton ");
    display.setCursor(0, 20);
    display.print(tecla);
    display.sendBuffer(); // Enviar al display
}
