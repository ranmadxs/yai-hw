#ifndef YaiUltrasonicSensor_h
#define YaiUltrasonicSensor_h

#include <Arduino.h>
#include <PubSubClient.h>

// Constantes para cálculos
// Velocidad del sonido en el aire aprox 0.0343 cm/microsegundo
const float VELOCIDAD_SONIDO = 0.0343;

class YaiUltrasonicSensor {
  public:
    YaiUltrasonicSensor(int pinTrig, int pinEcho, unsigned long measurementInterval = 500);
    
    void begin();
    void loop();
    void setMqttClient(PubSubClient* client);
    void setMqttTopic(const char* topic);
    
    float getDistance();
    String getStatus();
    unsigned long getLastMeasurement();
    bool isMqttEnabled();
    
  private:
    int pinTrig;
    int pinEcho;
    unsigned long measurementInterval;
    unsigned long lastMeasurement;
    
    float currentDistance;
    String currentStatus;
    
    PubSubClient* clientMqtt;
    const char* mqttTopic;
    bool mqttEnabled;
    
    void readSensor();
    void sendDataToMqtt();
    void calculateDistance(long duration);
};

#endif

