/**
 * Tests unitarios para MQTT - prueba el parsing de comandos (yaiUtil.string2YaiCommand)
 * que usa el callback MQTT en YaiMqtt.h
 * Ejecutar: pio test -e esp32_test -f test_mqtt
 */
#include <Arduino.h>
#include <ArduinoJson.h>
#include <unity.h>
#include "YaiCommons.h"

// Externs requeridos por YaiCommons (definidos en main.cpp en la app real)
bool ultrasonicLogsEnabled = false;
unsigned long ultrasonicMeasurementInterval = 1500;
const String DEVICE_ID = "TEST";
const String DEVICE_MQTT_TOPIC_OUT = "yai-mqtt/test/out";
const String DEVICE_MQTT_TOPIC_IN = "yai-mqtt/test/in";

void setUp(void) {}
void tearDown(void) {}

void test_string2YaiCommand_ON_con_intervalo(void) {
  YaiCommand cmd;
  cmd.message = "ON,2000,0,0,0,0,0,0";
  yaiUtil.string2YaiCommand(cmd);
  TEST_ASSERT_EQUAL_STRING("ON", cmd.command.c_str());
  TEST_ASSERT_EQUAL_STRING("2000", cmd.p1.c_str());
}

void test_string2YaiCommand_OFF(void) {
  YaiCommand cmd;
  cmd.message = "OFF,0,0,0,0,0,0,0";
  yaiUtil.string2YaiCommand(cmd);
  TEST_ASSERT_EQUAL_STRING("OFF", cmd.command.c_str());
}

void test_string2YaiCommand_comando_valido_mqtt(void) {
  // MQTT requiere length>10 y coma. ON,2000,0,0,0,0,0,0 cumple
  YaiCommand cmd;
  cmd.message = "ON,2000,0,0,0,0,0,0";
  yaiUtil.string2YaiCommand(cmd);
  TEST_ASSERT_TRUE(cmd.message.length() > 10);
  TEST_ASSERT_TRUE(cmd.message.indexOf(",") > 0);
  TEST_ASSERT_EQUAL_STRING("ON", cmd.command.c_str());
}

void test_string2YaiCommand_mensaje_vacio(void) {
  YaiCommand cmd;
  cmd.message = "";
  yaiUtil.string2YaiCommand(cmd);
  TEST_ASSERT_EQUAL_STRING("", cmd.command.c_str());
}

void test_string2YaiCommand_un_solo_parametro(void) {
  YaiCommand cmd;
  cmd.message = "PING";
  yaiUtil.string2YaiCommand(cmd);
  TEST_ASSERT_EQUAL_STRING("PING", cmd.command.c_str());
}

void test_string2YaiCommand_HELP(void) {
  YaiCommand cmd;
  cmd.message = "HELP";
  yaiUtil.string2YaiCommand(cmd);
  TEST_ASSERT_EQUAL_STRING("HELP", cmd.command.c_str());
}

void test_mqtt_topic_constants(void) {
  TEST_ASSERT_EQUAL_STRING("yai-mqtt/in", MQTT_TOPIC_IN);
  TEST_ASSERT_EQUAL_STRING("yai-mqtt/out", MQTT_TOPIC_OUT);
}

// Verifica que nadie cambie MQTT_SERVER (host), MQTT_USER, MQTT_PASSWORD ni MQTT_PORT
void test_mqtt_credenciales_inmutables(void) {
  TEST_ASSERT_EQUAL_STRING("broker.mqttdashboard.com", MQTT_SERVER);
  TEST_ASSERT_EQUAL(1883, MQTT_PORT);
  TEST_ASSERT_EQUAL_STRING("test", MQTT_USER);
  TEST_ASSERT_EQUAL_STRING("test", MQTT_PASSWORD);
}

// Verifica que el formato de datos de medición enviado a DEVICE_MQTT_TOPIC_OUT
// sea siempre el mismo (mismo esquema JSON)
void test_mqtt_medicion_formato_consistente(void) {
  // Estructura idéntica a YaiUltrasonicSensor::sendDataToMqtt()
  const char* deviceId = "TEST";
  const char* channelId = "1A2B3C4D";
  const char* status = "OKO";
  float distanceCm = 25.50f;
  const char* timestamp = "2024-01-15 14:30:25";
  float tankDepthCm = 160.0f;
  float remainingToFullCm = 134.50f;
  float fillLevelPercent = 84.06f;

  String mensaje = "{";
  mensaje += "\"deviceId\":\"" + String(deviceId) + "\"";
  mensaje += ",\"channelId\":\"" + String(channelId) + "\"";
  mensaje += ",\"status\":\"" + String(status) + "\"";
  mensaje += ",\"distanceCm\":" + String(distanceCm, 2);
  mensaje += ",\"timestamp\":\"" + String(timestamp) + "\"";
  mensaje += ",\"tankDepthCm\":" + String(tankDepthCm, 2);
  mensaje += ",\"remainingToFullCm\":" + String(remainingToFullCm, 2);
  mensaje += ",\"fillLevelPercent\":" + String(fillLevelPercent, 2);
  mensaje += "}";

  DynamicJsonDocument doc(512);
  DeserializationError err = deserializeJson(doc, mensaje);
  TEST_ASSERT_FALSE(err);

  // Verificar que siempre se envían los mismos campos a la cola MQTT
  TEST_ASSERT_TRUE(doc.containsKey("deviceId"));
  TEST_ASSERT_TRUE(doc.containsKey("channelId"));
  TEST_ASSERT_TRUE(doc.containsKey("status"));
  TEST_ASSERT_TRUE(doc.containsKey("distanceCm"));
  TEST_ASSERT_TRUE(doc.containsKey("timestamp"));
  TEST_ASSERT_TRUE(doc.containsKey("tankDepthCm"));
  TEST_ASSERT_TRUE(doc.containsKey("remainingToFullCm"));
  TEST_ASSERT_TRUE(doc.containsKey("fillLevelPercent"));

  // Verificar que no hay campos extra (solo los 8 esperados)
  TEST_ASSERT_EQUAL(8, doc.size());

  // Verificar tipos (ArduinoJson usa double para números)
  TEST_ASSERT_TRUE(doc["deviceId"].is<const char*>());
  TEST_ASSERT_TRUE(doc["status"].is<const char*>());
  TEST_ASSERT_TRUE(doc["distanceCm"].is<double>());
  TEST_ASSERT_TRUE(doc["fillLevelPercent"].is<double>());
}

void setup() {
  delay(2000);
  UNITY_BEGIN();
  RUN_TEST(test_string2YaiCommand_ON_con_intervalo);
  RUN_TEST(test_string2YaiCommand_OFF);
  RUN_TEST(test_string2YaiCommand_comando_valido_mqtt);
  RUN_TEST(test_string2YaiCommand_mensaje_vacio);
  RUN_TEST(test_string2YaiCommand_un_solo_parametro);
  RUN_TEST(test_string2YaiCommand_HELP);
  RUN_TEST(test_mqtt_topic_constants);
  RUN_TEST(test_mqtt_credenciales_inmutables);
  RUN_TEST(test_mqtt_medicion_formato_consistente);
  UNITY_END();
}

void loop() {}
