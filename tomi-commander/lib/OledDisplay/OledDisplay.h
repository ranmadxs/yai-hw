#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

#include <U8g2lib.h>

class OledDisplay {
public:
    OledDisplay();
    void iniciarPantalla();
    void mostrarMensaje(const char* mensaje);
    void mostrarTecla(char tecla);
private:
    U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C display;
};

#endif
