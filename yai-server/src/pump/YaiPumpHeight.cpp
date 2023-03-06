#include "YaiPumpHeight.h"
//#include "YaiContext.h"

void YaiPumpHeight::addCallback(YaiPumpHeightCallBack lolaso) {  
    callbacks[totalCallbacks] = lolaso;
    totalCallbacks++;

}

void YaiPumpHeight::setup() {
    // Ponemos el pin Trig en modo salida
    pinMode(PIN_TRIG, OUTPUT);
    // Ponemos el pin Echo en modo entrada
    pinMode(PIN_ECHO, INPUT);
    t0 = millis();
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
    this->num_ciclos = this->num_ciclos + 1;
    this->num_ciclos_vel++;

    if ( this->num_ciclos_vel == 0 || this->num_ciclos_vel == 6000) {
      this->t1 = millis();
      this->dt = this->t1 - this->t0;
      this->t0 = this->t1;
      iniciarTrigger();
      // La función pulseIn obtiene el tiempo que tarda en cambiar entre estados, en este caso a HIGH
      unsigned long tiempo = pulseIn(PIN_ECHO, HIGH);
    
      // Obtenemos la distancia en cm, hay que convertir el tiempo en segudos ya que está en microsegundos
      // por eso se multiplica por 0.000001
      this->distancia = tiempo * 0.000001 * VEL_SONIDO / 2.0;
      float heigthH2OTank = HEIGHT_MAX + HEIGHT_OFFSET - (this->distancia);
      float vol = TANK_MAX_VOL * (heigthH2OTank - HEIGHT_OFFSET) / HEIGHT_MAX;

      if(this->num_ciclos_vel == 0) {
        this->x0 = vol;
      }
      if(this->num_ciclos_vel == 6000) {
        this->x1 = vol;
        this->num_ciclos_vel = 0;
        this->v0 = (this->x1 - this->x0)/this->dt;
      }
      if(this->num_ciclos == 6000){
        this->num_ciclos = 0;
        String logMsg = String(this->distancia) + "cm";
        DynamicJsonDocument  doc(500);
        DynamicJsonDocument docValueInfo(400);
        docValueInfo = *(docInfo);
        doc["type"] = "YAI_TANK_HEIGHT";
        doc["distance"] = this->distancia;
        doc["water_flow"] = this->v0 / 60;
        doc["time"] = this->dt;
        doc["height"] = heigthH2OTank;
        doc["height_max"] = HEIGHT_MAX;
        doc["volume"] = vol;
        doc["info"] = (*docInfo);
        /*String msg = "{ \"type\":\"YAI_TANK_HEIGHT\", \"distance\": " + String(this->distancia) 
          + ", \"water_flow\": "+String(this->v0)+", \"delta_time\": "+String(this->dt)
          + ", \"height_max\": "+ HEIGHT_MAX+", \"height\": "+heigthH2OTank+", \"volume\": "+vol+"}";
        */
        String stccall = "";
        serializeJson(doc, stccall);
        for (int i=0; i < this->totalCallbacks; i++) {
          callbacks[i].function(stccall); 
        }
        this->logger.debug(stccall);
      }
    }
}