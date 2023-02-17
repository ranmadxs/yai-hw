#include "YaiPumpHeight.h"
//#include "YaiContext.h"

void YaiPumpHeight::setup() {
    // Ponemos el pin Trig en modo salida
    pinMode(PIN_TRIG, OUTPUT);
    // Ponemos el pin Echo en modo entrada
    pinMode(PIN_ECHO, INPUT);
    Serial.println(" ######### YAI PUMP H [OK] ###########");
}

// Método que inicia la secuencia del Trigger para comenzar a medir
void YaiPumpHeight::iniciarTrigger() {
  // Ponemos el Triiger en estado bajo y esperamos 2 ms
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  
  // Ponemos el pin Trigger a estado alto y esperamos 10 ms
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  
  // Comenzamos poniendo el pin Trigger en estado bajo
  digitalWrite(PIN_TRIG, LOW);

}


void YaiPumpHeight::loop() {
    iniciarTrigger();
    // La función pulseIn obtiene el tiempo que tarda en cambiar entre estados, en este caso a HIGH
    unsigned long tiempo = pulseIn(PIN_ECHO, HIGH);
  
    // Obtenemos la distancia en cm, hay que convertir el tiempo en segudos ya que está en microsegundos
    // por eso se multiplica por 0.000001
    this->distancia = tiempo * 0.000001 * VEL_SONIDO / 2.0;
    String logMsg = String(distancia) + "cm";
    this->logger.debug(logMsg);
    //Serial.print(this->distancia);
    //Serial.print("cm");
    //Serial.println();
    delay(500);
}