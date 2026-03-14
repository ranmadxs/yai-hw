#ifndef YaiUltrasonicSensor_h
#define YaiUltrasonicSensor_h

#include <Arduino.h>
#include <PubSubClient.h>

// Status constants from YaiCommons (using same base definitions)
#ifndef STATUS_OK
#define STATUS_OK "OKO"
#endif
#ifndef STATUS_NOK
#define STATUS_NOK "NOK"
#endif

// Device ID constant (will be defined externally in main.cpp)
extern const String DEVICE_ID;
extern const String DEVICE_MQTT_TOPIC_OUT;
extern const String DEVICE_MQTT_TOPIC_IN;

// Global control variables
extern bool ultrasonicLogsEnabled;
extern unsigned long ultrasonicMeasurementInterval;

// Función para obtener timestamp formateado
extern String getCurrentTimestamp();

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
    void setChannelId(String id);

    float getDistance();
    String getStatus();
    unsigned long getLastMeasurement();
    bool isMqttEnabled();

  private:
    int pinTrig;
    int pinEcho;
    unsigned long lastMeasurement;

    float currentDistance;
    String currentStatus;
    String channelId;

    PubSubClient* clientMqtt;
    const char* mqttTopic;
    bool mqttEnabled;

    void readSensor();
    void sendDataToMqtt();
    void calculateDistance(long duration);
};

#endif

