#include "YaiWebServer.h"
#include <ArduinoJson.h>

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#endif

static const unsigned long WIFI_CACHE_TTL_MS = 30000;  // 30 s

YaiWebServer::YaiWebServer() : _server(80), _cacheValidUntil(0), _scanInProgress(false), _wifiRestoreCb(nullptr), _wifiSaveCb(nullptr) {}

void YaiWebServer::begin() {
  _server.on("/api/wifi", HTTP_GET, [this]() { _handleGetWifi(); });
  _server.on("/api/wifi", HTTP_POST, [this]() { _handlePostWifi(); });
  _server.onNotFound([this]() { _handleNotFound(); });
  _server.begin();
  _startAsyncScan();  // Scan en background al iniciar
  Serial.println("WebServer >> listening on port 80, /api/wifi GET|POST");
}

void YaiWebServer::loop() {
  _updateScanCache();
  _server.handleClient();
}

void YaiWebServer::_startAsyncScan() {
  if (_scanInProgress) return;
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.disconnect();
    delay(50);
  }
#if defined(ESP8266)
  WiFi.scanNetworks(true);
#else
  WiFi.scanNetworks(true, false);
#endif
  _scanInProgress = true;
}

void YaiWebServer::_updateScanCache() {
  if (_scanInProgress) {
    int n = WiFi.scanComplete();
    if (n < 0) return;  // -1: running, -2: not started
    _scanInProgress = false;
    _cachedWifiJson = _buildWifiJson(n);
    _cacheValidUntil = millis() + WIFI_CACHE_TTL_MS;
    return;
  }
  // Refresh proactivo: iniciar nuevo scan 5 s antes de que expire el cache
  if (_cachedWifiJson.length() > 0 && millis() + 5000 >= _cacheValidUntil) {
    _startAsyncScan();
  }
}

String YaiWebServer::_buildWifiJson(int n) {
  DynamicJsonDocument doc(512 + n * 80);
  doc["connected_ssid"] = (WiFi.status() == WL_CONNECTED) ? WiFi.SSID() : "";
  doc["connected_rssi"] = (WiFi.status() == WL_CONNECTED) ? WiFi.RSSI() : 0;
  JsonArray networks = doc.createNestedArray("networks");
  for (int i = 0; i < n; i++) {
    JsonObject net = networks.createNestedObject();
    net["ssid"] = WiFi.SSID(i);
    net["rssi"] = WiFi.RSSI(i);
#if defined(ESP8266)
    net["encryption"] = (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? "open" : "encrypted";
#else
    net["encryption"] = (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "open" : "encrypted";
#endif
  }
  String json;
  serializeJson(doc, json);
  return json;
}

void YaiWebServer::_handleGetWifi() {
  // Cache válido: respuesta inmediata
  if (_cachedWifiJson.length() > 0 && millis() < _cacheValidUntil) {
    _server.send(200, "application/json", _cachedWifiJson);
    return;
  }
  // Scan en curso: pedir retry
  if (_scanInProgress) {
    _server.send(200, "application/json", "{\"scanning\":true,\"retry_after\":2}");
    return;
  }
  // Sin cache: iniciar scan async y pedir retry
  _startAsyncScan();
  _server.send(200, "application/json", "{\"scanning\":true,\"retry_after\":2}");
}

/**
 * Cliente de prueba: intenta conectar SOLO con el ssid/password del POST.
 * No usa la lista hardcodeada. Si falla, restaura la red anterior.
 * Retorna true si conectó, false si no.
 */
bool YaiWebServer::_testWifiCredentials(const char* ssid, const char* password) {
  Serial.println("WebServer >> Probando conexión a: " + String(ssid) + " (credenciales del POST)");

  // Asegurar modo AP+STA: AP para el WebServer, STA para la prueba
#if defined(ESP32)
  WiFi.mode(WIFI_AP_STA);
#endif
  WiFi.disconnect();
  delay(200);
  // Red abierta (sin clave): usar WiFi.begin(ssid). "" falla en ESP32/ESP8266.
  if (password == nullptr || strlen(password) == 0) {
    WiFi.begin(ssid);
  } else {
    WiFi.begin(ssid, password);
  }

  int timeout = 400;  // 40 s
  while (WiFi.status() != WL_CONNECTED && timeout-- > 0) {
    delay(100);
  }

  bool ok = (WiFi.status() == WL_CONNECTED);

  if (ok) {
    Serial.println("WebServer >> Test OK - conectado a " + String(ssid) + " (credenciales del POST)");
    // Mantener conexión: no restaurar, la red del usuario pasa a ser la principal
  } else {
#if defined(ESP32)
    // Log del motivo del fallo para depurar
    wl_status_t st = WiFi.status();
    const char* reason = (st == WL_NO_SSID_AVAIL) ? "red no encontrada" :
                         (st == WL_CONNECT_FAILED) ? "credenciales inválidas" :
                         (st == WL_CONNECTION_LOST) ? "conexión perdida" :
                         (st == WL_DISCONNECTED) ? "timeout (no completó en 40s)" : "otro";
    Serial.println("WebServer >> Test falló para " + String(ssid) + " (" + String(reason) + ", status=" + String((int)st) + ") - restaurando lista hardcodeada");
#else
    Serial.println("WebServer >> Test falló para " + String(ssid) + " - restaurando lista hardcodeada");
#endif
    WiFi.disconnect();
    delay(100);
    if (_wifiRestoreCb) _wifiRestoreCb();
  }
  return ok;
}

void YaiWebServer::_handlePostWifi() {
  if (_server.hasArg("plain") == false) {
    _server.send(400, "application/json", "{\"error\":\"Body JSON required\"}");
    return;
  }

  StaticJsonDocument<256> doc;
  DeserializationError err = deserializeJson(doc, _server.arg("plain"));
  if (err) {
    _server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }

  const char* ssid = doc["ssid"];
  const char* password = doc["password"] | doc["pass"] | "";

  Serial.println("WebServer >> POST /api/wifi - ssid: " + String(ssid) + ", password: " + String(password));

  if (!ssid || strlen(ssid) == 0) {
    _server.send(400, "application/json", "{\"error\":\"ssid required\"}");
    return;
  }

  // Usar cliente de prueba: no modifica la conexión principal, solo valida credenciales
  bool ok = _testWifiCredentials(ssid, password);

  if (ok) {
    Serial.println("WebServer >> guardado bien");
    if (_wifiSaveCb) _wifiSaveCb(ssid, password);
    _server.send(200, "application/json", "{\"ok\":true,\"message\":\"Conectado a " + String(ssid) + "\"}");
  } else {
    Serial.println("WebServer >> guardado mal");
    StaticJsonDocument<128> errDoc;
    errDoc["error"] = "No se pudo conectar a " + String(ssid);
    String errJson;
    serializeJson(errDoc, errJson);
    _server.send(400, "application/json", errJson);
  }
}

void YaiWebServer::_handleNotFound() {
  _server.send(404, "application/json", "{\"error\":\"Not found\"}");
}
