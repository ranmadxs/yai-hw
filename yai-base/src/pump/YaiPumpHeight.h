#ifndef YaiPumpHeight_h
#define YaiPumpHeight_h
#include <Arduino.h>
#include "YaiCommons.hpp"
// Configuramos los pines del sensor Trigger y Echo
const int PIN_TRIG = 5;   //ESP_D1
const int PIN_ECHO = 4;   //ESP_D2


// Constante velocidad sonido en cm/s
const float VEL_SONIDO = 34000.0;


class YaiPumpHeight {
  public:
    YaiPumpHeight(){};
    YaiPumpHeight(YaiLog _logger){
        this->logger = _logger;
    };
    void setup();
    void loop();
    void setLogger(YaiLog _logger) {
        this->logger = _logger;
    }

  private:
    YaiLog logger;
    float distancia;
    void iniciarTrigger();
};

#endif
