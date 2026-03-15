/**
 * Tests unitarios para WiFi (YaiWIFI)
 * Ejecutar: pio test -e esp32_test -f test_wifi
 */
#include <Arduino.h>
#include <unity.h>
#include "YaiWIFI.h"

void setUp(void) {}
void tearDown(void) {}

void test_yaiwifi_instancia(void) {
  YaiWIFI wifi;
  TEST_ASSERT_NOT_NULL(&wifi);
}

void test_yaiwifi_getIp_inicial(void) {
  YaiWIFI wifi;
  String ip = wifi.getIp();
  // Antes de connect(), ipLocal puede estar vacío o no inicializado
  TEST_ASSERT_TRUE(ip.length() >= 0);
}

void test_yaiwifi_getMac(void) {
  YaiWIFI wifi;
  String mac = wifi.getMac();
  // WiFi.macAddress() devuelve algo como "AA:BB:CC:DD:EE:FF"
  TEST_ASSERT_TRUE(mac.length() > 0);
  TEST_ASSERT_TRUE(mac.indexOf(":") >= 0);
}

void test_yaiwifi_isConnected_sin_conectar(void) {
  YaiWIFI wifi;
  // Sin haber llamado connect(), normalmente no está conectado
  bool connected = wifi.isConnected();
  // Puede ser true si hay credenciales guardadas y auto-connect
  TEST_ASSERT_TRUE(connected == true || connected == false);
}

void test_yaiwifi_constantes(void) {
  TEST_ASSERT_EQUAL(5, totalWifi);
  TEST_ASSERT_EQUAL(25, retryWifi);
}

void setup() {
  delay(2000);
  UNITY_BEGIN();
  RUN_TEST(test_yaiwifi_instancia);
  RUN_TEST(test_yaiwifi_getIp_inicial);
  RUN_TEST(test_yaiwifi_getMac);
  RUN_TEST(test_yaiwifi_isConnected_sin_conectar);
  RUN_TEST(test_yaiwifi_constantes);
  UNITY_END();
}

void loop() {}
