#include "YaiUltrasonicSensor.h"
#include "YaiHttpClient.h"
#include "YaiUdpDiscovery.h"

// External declarations
extern const String DEVICE_ID;
extern const String CHANNEL_ID;
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
  this->httpClient = nullptr;
  this->udpDiscovery = nullptr;
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

void YaiUltrasonicSensor::setHttpClient(YaiHttpClient* client) {
  this->httpClient = client;
}

void YaiUltrasonicSensor::setUdpDiscovery(YaiUdpDiscovery* udp) {
  this->udpDiscovery = udp;
}

void YaiUltrasonicSensor::loop() {
  // Medir solo cuando hay destinatario: evita bloqueo de pulseIn durante discovery
  bool tieneDestinatario = ultrasonicLogsEnabled ||
    (udpDiscovery && udpDiscovery->hasSubscriber()) ||
    (httpClient != nullptr) ||
    (mqttEnabled && clientMqtt != nullptr);
  if (!tieneDestinatario) return;

  unsigned long currentTime = millis();
  if (currentTime - lastMeasurement >= ultrasonicMeasurementInterval) {
    lastMeasurement = currentTime;
    readSensor();
    publishReading();
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
  // Timeout 60ms: suficiente para 400cm (~24ms), evita bloqueos de 1s cuando no hay eco
  long duration = pulseIn(pinEcho, HIGH, 60000);

  // 4. Calculamos la distancia y determinamos el estado
  calculateDistance(duration);

  // 5. Imprimimos resultados por serial solo si logs están habilitados
  if (ultrasonicLogsEnabled) {
    Serial.print(DEVICE_ID + "@" + CHANNEL_ID + " % ");
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

// Profundidad del tanque en cm definida en main.cpp
extern const float TANK_DEPTH_CM;

// Offset del sensor (cm por encima del nivel de referencia), definido en main.cpp
extern const float SENSOR_HEIGHT_OFFSET_CM;

void YaiUltrasonicSensor::calculateDistance(long duration) {
  // Distancia = (Tiempo * Velocidad) / 2 (porque el sonido va y vuelve)
  float rawDistance = (duration * VELOCIDAD_SONIDO) / 2;
  // Corregir: el sensor está SENSOR_HEIGHT_OFFSET_CM por encima del nivel de referencia
  currentDistance = rawDistance - SENSOR_HEIGHT_OFFSET_CM;
  if (currentDistance < 0) currentDistance = 0;

  // duration==0 → timeout (sin eco). Fuera de 2-400 cm → inválido (ruido, cable suelto, o fuera de rango)
  if (duration == 0 || rawDistance < DISTANCIA_MIN_CM || rawDistance > DISTANCIA_MAX_CM) {
    currentStatus = String(STATUS_NOK);
  } else {
    currentStatus = String(STATUS_OK);
  }
}

void YaiUltrasonicSensor::publishReading() {
  // Publicar a MQTT, HTTP y/o UDP cuando hay subscriber o cliente configurado
  if (!udpDiscovery && !httpClient && !(mqttEnabled && clientMqtt != nullptr)) return;

  // Formato JSON para datos del sensor:
    // {
    //   "deviceId": "...",
    //   "status": "OKO" | "NOK",
    //   "distanceCm": 25.50,
    //   "timestamp": "2024-01-15 14:30:25",
    //   "tankDepthCm": 160.0,
    //   "remainingToFullCm": 134.50,
    //   "fillLevelPercent": 84.06
    // }
    String timestamp = getCurrentTimestamp();

    float distance = currentDistance;
    if (distance < 0) {
      distance = 0;
    }
    // Altura de líquido = profundidad - distancia medida desde el sensor al líquido
    float filledHeightCm = TANK_DEPTH_CM - distance;
    if (filledHeightCm < 0) {
      filledHeightCm = 0;
    }
    if (filledHeightCm > TANK_DEPTH_CM) {
      filledHeightCm = TANK_DEPTH_CM;
    }

    float remainingToFullCm = TANK_DEPTH_CM - filledHeightCm;
    if (remainingToFullCm < 0) {
      remainingToFullCm = 0;
    }

    float fillLevelPercent = 0.0;
    if (TANK_DEPTH_CM > 0) {
      fillLevelPercent = (filledHeightCm / TANK_DEPTH_CM) * 100.0;
    }

    // Formato JSON: 8 campos (test_mqtt_medicion_formato_consistente verifica este esquema)
    String mensaje = "{";
    mensaje += "\"deviceId\":\"" + DEVICE_ID + "\"";
    mensaje += ",\"channelId\":\"" + CHANNEL_ID + "\"";
    mensaje += ",\"status\":\"" + currentStatus + "\"";
    mensaje += ",\"distanceCm\":" + String(distance, 2);
    mensaje += ",\"timestamp\":\"" + timestamp + "\"";
    mensaje += ",\"tankDepthCm\":" + String(TANK_DEPTH_CM, 2);
    mensaje += ",\"remainingToFullCm\":" + String(remainingToFullCm, 2);
    mensaje += ",\"fillLevelPercent\":" + String(fillLevelPercent, 2);
    mensaje += "}";

  // Enviar por MQTT si está habilitado
  if (mqttEnabled && clientMqtt != nullptr && clientMqtt->connected() && mqttTopic != nullptr) {
    clientMqtt->publish(DEVICE_MQTT_TOPIC_OUT.c_str(), mensaje.c_str());
  }

  // Acumular para envío HTTP batch (cada 1 min)
  if (httpClient != nullptr) {
    httpClient->addReading(mensaje);
  }

  // Enviar por UDP al subscriber (topic real: yai-mqtt/<CHANNEL_ID>/out)
  if (udpDiscovery != nullptr) {
    udpDiscovery->sendToTopic(DEVICE_MQTT_TOPIC_OUT.c_str(), mensaje);
  }

  // Mostrar por Serial solo si logs están habilitados
  if (ultrasonicLogsEnabled) {
    Serial.print(DEVICE_ID + "@" + CHANNEL_ID + " % ");
    Serial.println("MQTT >> " + mensaje);
  }
}

void YaiUltrasonicSensor::sendDataToMqtt() {
  publishReading();
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

