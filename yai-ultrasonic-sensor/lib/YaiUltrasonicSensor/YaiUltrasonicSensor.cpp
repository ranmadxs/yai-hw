#include "YaiUltrasonicSensor.h"
#include "YaiCommons.h"

YaiUltrasonicSensor::YaiUltrasonicSensor(int pinTrig, int pinEcho, unsigned long measurementInterval) {
  this->pinTrig = pinTrig;
  this->pinEcho = pinEcho;
  this->measurementInterval = measurementInterval;
  this->lastMeasurement = 0;
  this->currentDistance = 0.0;
  this->currentStatus = STATUS_NOK;
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
  unsigned long currentTime = millis();
  if (currentTime - lastMeasurement >= measurementInterval) {
    lastMeasurement = currentTime;
    readSensor();
    if (mqttEnabled && mqttTopic != nullptr) {
      sendDataToMqtt();
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
  
  // 5. Imprimimos resultados por serial
  Serial.print("Distancia: ");
  Serial.print(currentDistance);
  Serial.print(" cm | Estado: ");
  Serial.print(currentStatus);
  Serial.print(" | Tiempo: ");
  Serial.print(millis());
  Serial.println(" ms");
}

void YaiUltrasonicSensor::calculateDistance(long duration) {
  // Distancia = (Tiempo * Velocidad) / 2 (porque el sonido va y vuelve)
  currentDistance = (duration * VELOCIDAD_SONIDO) / 2;
  
  // Determinamos el estado del sensor
  if (duration == 0 || currentDistance <= 0 || currentDistance > 400) {
    // Rango típico del sensor es 2-400 cm
    currentStatus = STATUS_NOK;
  } else {
    currentStatus = STATUS_OK;
  }
}

void YaiUltrasonicSensor::sendDataToMqtt() {
  if (mqttEnabled && clientMqtt != nullptr && mqttTopic != nullptr && clientMqtt->connected()) {
    // Formato: ULTRASONIC,ESTADO,DISTANCIA,TIMESTAMP
    // Ejemplo: ULTRASONIC,OKO,25.5,12345
    unsigned long systemTime = millis();
    String mensaje = "ULTRASONIC," + currentStatus + "," + String(currentDistance, 2) + "," + String(systemTime);
    
    // Enviamos el mensaje al topic OUT
    clientMqtt->publish(mqttTopic, mensaje.c_str());
    
    Serial.println("MQTT >> " + mensaje);
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

