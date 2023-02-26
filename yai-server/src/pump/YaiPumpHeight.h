#ifndef YaiPumpHeight_h
#define YaiPumpHeight_h
#include <Arduino.h>
#include "YaiCommons.hpp"
#include "YaiContext.h"
//#include <YaiWebSocket.h>
//https://helloworld.co.in/article/smart-water-tank
//https://codepen.io/niamleeson/pen/KgRpvb
class YaiPumpHeightCallBack {
  public:
    ~YaiPumpHeightCallBack(){}
    YaiPumpHeightCallBack(void (*f)(String) = 0)
        : function(f) {}
    void (*function)(String);
};

class YaiPumpHeight {
  public:
    YaiPumpHeight(){};
    void setup();
    void loop();
    void setLogger(YaiLog _logger) {
        this->logger = _logger;
    }
    void setMqtt(PubSubClient _clientMqtt) {
      this->clientMqtt = _clientMqtt;
    }
    void addCallback(YaiPumpHeightCallBack lolaso);

  private:
    YaiLog logger;
    unsigned long t0 = 0;
    unsigned long t1 = 0;
    unsigned long dt = 0;
    int num_ciclos = 0;
    PubSubClient clientMqtt;
    float distancia;
    void iniciarTrigger();
    YaiPumpHeightCallBack callbacks[10];
    int totalCallbacks = 0;
    // Configuramos los pines del sensor Trigger y Echo
    const int PIN_TRIG = 5;   //ESP_D1
    const int PIN_ECHO = 4;   //ESP_D2

    //https://helloworld.co.in/article/smart-water-tank
    // Constante velocidad sonido en cm/s
    const float VEL_SONIDO = 34000.0;
    const float HEIGHT_MAX = 100;
    const float HEIGHT_OFFSET = 10;
    const float TANK_MAX_VOL = 1000;   
};

#endif
