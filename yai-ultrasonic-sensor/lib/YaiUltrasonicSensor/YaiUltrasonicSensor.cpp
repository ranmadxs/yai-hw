#include "YaiUltrasonicSensor.h"

// External declarations
extern const String DEVICE_ID;
extern const String DEVICE_MQTT_TOPIC_OUT;
extern bool ultrasonicLogsEnabled;
extern unsigned long ultrasonicMeasurementInterval;

YaiUltrasonicSensor::YaiUltrasonicSensor(int pinTrig, int pinEcho, unsigned long measurementInterval) {
  this->pinTrig = pinTrig;
  this->pinEcho = pinEcho;
  this->lastMeasurement = 0;
  this->currentDistance = 0.0;
  this->currentStatus = String(STATUS_NOK);
  this->clientMqtt = nullptr;
  this->mqttTopic = nullptr;
  this->mqttEnabled = false;
}

void YaiUltrasonicSensor::begin() {
  // Configuramos los pines del sensor ultrasónico
  pinMode(pinTrig, OUTPUT); // El trigger emite sonido (Salida)
  pinMode(pinEcho, INPUT);  // El echo escucha el rebote (Entrada)
  
  Serial.println("Iniciando sensor AJ-SR04M...");
}

void YaiUltrasonicSensor::setMqttClient(PubSubClient* client) {
  this->clientMqtt = client;
  this->mqttEnabled = (client != nullptr);
}

void YaiUltrasonicSensor::setMqttTopic(const char* topic) {
  this->mqttTopic = topic;
}

void YaiUltrasonicSensor::loop() {
  if (ultrasonicLogsEnabled) {
    unsigned long currentTime = millis();
    if (currentTime - lastMeasurement >= ultrasonicMeasurementInterval) {
      lastMeasurement = currentTime;
      readSensor();
      if (mqttEnabled && mqttTopic != nullptr) {
        sendDataToMqtt();
      }
    }
  }
}

void YaiUltrasonicSensor::readSensor() {
  // 1. Limpiamos el pin Trig
  digitalWrite(pinTrig, LOW);
  delayMicroseconds(2);

  // 2. Generamos un pulso de 10 microsegundos
  digitalWrite(pinTrig, HIGH);
  delayMicroseconds(10);
  digitalWrite(pinTrig, LOW);

  // 3. Leemos el tiempo que tarda el sonido en volver (en microsegundos)
  // pulseIn espera a que el pin pase a HIGH y cuenta el tiempo
  long duration = pulseIn(pinEcho, HIGH);

  // 4. Calculamos la distancia y determinamos el estado
  calculateDistance(duration);

  // 5. Imprimimos resultados por serial solo si logs están habilitados
  if (ultrasonicLogsEnabled) {
    Serial.print("Distancia: ");
    Serial.print(currentDistance);
    Serial.print(" cm | Estado: ");
    Serial.print(currentStatus);
    Serial.print(" | Tiempo: ");
    Serial.print(millis());
    Serial.println(" ms");
  }
}

// Rango útil del sensor AJ-SR04M: 2-400 cm (por debajo de 2 cm = ruido o cable desconectado)
static const float DISTANCIA_MIN_CM = 2.0;
static const float DISTANCIA_MAX_CM = 400.0;

void YaiUltrasonicSensor::calculateDistance(long duration) {
  // Distancia = (Tiempo * Velocidad) / 2 (porque el sonido va y vuelve)
  currentDistance = (duration * VELOCIDAD_SONIDO) / 2;
  
  // duration==0 → timeout (sin eco). Fuera de 2-400 cm → inválido (ruido, cable suelto, o fuera de rango)
  if (duration == 0 || currentDistance < DISTANCIA_MIN_CM || currentDistance > DISTANCIA_MAX_CM) {
    currentStatus = String(STATUS_NOK);
  } else {
    currentStatus = String(STATUS_OK);
  }
}

void YaiUltrasonicSensor::sendDataToMqtt() {
  if (mqttEnabled && clientMqtt != nullptr && clientMqtt->connected()) {
    // Formato: DEVICE_ID,ESTADO,DISTANCIA,TIMESTAMP
    // Ejemplo: YUS-0.2.7-RC-COSTA,OKO,25.5,2024-01-15 14:30:25
    String timestamp = getCurrentTimestamp();
    String mensaje = DEVICE_ID + "," + currentStatus + "," + String(currentDistance, 2) + "," + timestamp;

    // Enviamos SOLO al canal específico del dispositivo (NO al canal general)
    clientMqtt->publish(DEVICE_MQTT_TOPIC_OUT.c_str(), mensaje.c_str());

    // Mostramos mensaje MQTT solo si logs están habilitados
    if (ultrasonicLogsEnabled) {
      Serial.println("MQTT >> " + mensaje + " (enviado a " + DEVICE_MQTT_TOPIC_OUT + ")");
    }
  }
}

float YaiUltrasonicSensor::getDistance() {
  return currentDistance;
}

String YaiUltrasonicSensor::getStatus() {
  return currentStatus;
}

unsigned long YaiUltrasonicSensor::getLastMeasurement() {
  return lastMeasurement;
}

bool YaiUltrasonicSensor::isMqttEnabled() {
  return mqttEnabled;
}

