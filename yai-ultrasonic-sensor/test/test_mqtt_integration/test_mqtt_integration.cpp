/**
 * Tests de integración MQTT - conecta al broker real.
 * Requiere: WiFi disponible, broker broker.mqttdashboard.com accesible.
 * Ejecutar: pio test -e esp32_test -f test_mqtt_integration
 */
#include <Arduino.h>
#include <unity.h>
#include <PubSubClient.h>
#include "YaiCommons.h"
#include "YaiMqtt.h"

bool ultrasonicLogsEnabled = false;
bool udpDiscoveryLogsEnabled = false;
unsigned long ultrasonicMeasurementInterval = 1500;
const String DEVICE_ID = "TEST";
extern const String CHANNEL_ID = "TEST1234";
const String DEVICE_MQTT_TOPIC_OUT = "yai-mqtt/" + CHANNEL_ID + "/out";
const String DEVICE_MQTT_TOPIC_IN = "yai-mqtt/" + CHANNEL_ID + "/in";

static bool pongReceived = false;

void setUp(void) {
  pongReceived = false;
}
void tearDown(void) {}

static bool connectWifiAndMqtt() {
  if (!yaiWifi.isConnected()) {
    yaiWifi.connect();
  }
  if (!yaiWifi.isConnected()) return false;
  clientMqtt.setServer(MQTT_SERVER, MQTT_PORT);
  return clientMqtt.connect(("YaiTest-" + String(random(0xffff), HEX)).c_str(), MQTT_USER, MQTT_PASSWORD);
}

void test_mqtt_hello_world(void) {
  Serial.println("Test hello world: conectando...");
  if (!connectWifiAndMqtt()) {
    TEST_IGNORE_MESSAGE("WiFi no conectado");
    return;
  }

  String clientId = "YaiTest-" + String(random(0xffff), HEX);
  String helloMsg = "hello world " + clientId;
  bool published = clientMqtt.publish(MQTT_TOPIC_OUT, helloMsg.c_str());
  clientMqtt.disconnect();

  TEST_ASSERT_TRUE_MESSAGE(published, "No se pudo publicar hello world en MQTT_TOPIC_OUT.");
}

void test_mqtt_ping_pong(void) {
  Serial.println("Test PING/PONG: conectando...");
  if (!connectWifiAndMqtt()) {
    TEST_IGNORE_MESSAGE("WiFi no conectado");
    return;
  }

  clientMqtt.setCallback(callbackMqtt);
  clientMqtt.subscribe(MQTT_TOPIC_IN);
  clientMqtt.subscribe(DEVICE_MQTT_TOPIC_IN.c_str());

  WiFiClient testClient;
  PubSubClient testMqtt(testClient);
  testMqtt.setServer(MQTT_SERVER, MQTT_PORT);
  String testId = "YaiPingTest-" + String(random(0xffff), HEX);
  if (!testMqtt.connect(testId.c_str(), MQTT_USER, MQTT_PASSWORD)) {
    clientMqtt.disconnect();
    TEST_FAIL_MESSAGE("No se pudo conectar cliente de prueba");
    return;
  }

  testMqtt.setCallback([](char* topic, byte* payload, unsigned int length) {
    String msg;
    for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];
    if (msg.startsWith("PONG")) pongReceived = true;
  });
  testMqtt.subscribe(MQTT_TOPIC_OUT);

  testMqtt.publish(MQTT_TOPIC_IN, "PING");

  for (int i = 0; i < 50; i++) {
    clientMqtt.loop();
    testMqtt.loop();
    delay(100);
    if (pongReceived) break;
  }

  testMqtt.disconnect();
  clientMqtt.disconnect();

  TEST_ASSERT_TRUE_MESSAGE(pongReceived, "No se recibió PONG tras enviar PING.");
}

void test_mqtt_conexion_y_publicar_medicion(void) {
  Serial.println("Test integración: conectando WiFi...");
  yaiWifi.connect();

  if (!yaiWifi.isConnected()) {
    Serial.println("SKIP: WiFi no disponible. Conecta a una red y vuelve a ejecutar.");
    TEST_IGNORE_MESSAGE("WiFi no conectado - test de integración requiere red");
    return;
  }

  Serial.println("WiFi OK. Conectando a MQTT broker " + String(MQTT_SERVER) + ":" + String(MQTT_PORT) + "...");
  clientMqtt.setServer(MQTT_SERVER, MQTT_PORT);

  String clientId = "YaiTest-" + String(random(0xffff), HEX);
  bool connected = clientMqtt.connect(
    clientId.c_str(),
    MQTT_USER,
    MQTT_PASSWORD
  );

  TEST_ASSERT_TRUE_MESSAGE(connected, "No se pudo conectar al broker MQTT. Verifica red y que broker.mqttdashboard.com esté accesible.");
  if (!connected) return;

  Serial.println("MQTT conectado OK. Publicando medición en " + DEVICE_MQTT_TOPIC_OUT + "...");

  // Formato idéntico a YaiUltrasonicSensor::sendDataToMqtt()
  const float TANK_DEPTH_CM = 160.0f;
  float distanceCm = 25.50f;
  float filledHeightCm = TANK_DEPTH_CM - distanceCm;
  float remainingToFullCm = TANK_DEPTH_CM - filledHeightCm;
  float fillLevelPercent = (filledHeightCm / TANK_DEPTH_CM) * 100.0f;

  String mensaje = "{";
  mensaje += "\"deviceId\":\"" + DEVICE_ID + "\"";
  mensaje += ",\"channelId\":\"" + CHANNEL_ID + "\"";
  mensaje += ",\"status\":\"OKO\"";
  mensaje += ",\"distanceCm\":" + String(distanceCm, 2);
  mensaje += ",\"timestamp\":\"2024-01-15 14:30:25\"";
  mensaje += ",\"tankDepthCm\":" + String(TANK_DEPTH_CM, 2);
  mensaje += ",\"remainingToFullCm\":" + String(remainingToFullCm, 2);
  mensaje += ",\"fillLevelPercent\":" + String(fillLevelPercent, 2);
  mensaje += "}";

  bool published = clientMqtt.publish(DEVICE_MQTT_TOPIC_OUT.c_str(), mensaje.c_str());

  clientMqtt.disconnect();

  TEST_ASSERT_TRUE_MESSAGE(published, "No se pudo publicar la medición en DEVICE_MQTT_TOPIC_OUT.");
}

void setup() {
  delay(2000);
  UNITY_BEGIN();
  RUN_TEST(test_mqtt_hello_world);
  RUN_TEST(test_mqtt_ping_pong);
  RUN_TEST(test_mqtt_conexion_y_publicar_medicion);
  UNITY_END();
}

void loop() {}
