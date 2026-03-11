#include "YaiHttpClient.h"
#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <ArduinoJson.h>

struct HttpTaskData {
  char url[256];
  char aiaOrigin[64];
  String* items;
  int n;
};

static void doHttpSend(HttpTaskData* d) {
  String endpoint = String(d->url);
  if (!endpoint.endsWith("/")) endpoint += "/";
  endpoint += "monitor/api/lecturas";

  String body = "{\"lecturas\":[";
  for (int i = 0; i < d->n; i++) {
    if (i > 0) body += ",";
    body += d->items[i];
  }
  body += "]}";

  HTTPClient http;
#if defined(ESP8266)
  BearSSL::WiFiClientSecure client;
  client.setInsecure();
  http.begin(client, endpoint);
#else
  http.begin(endpoint);
#endif
  http.addHeader("Content-Type", "application/json");
  if (d->aiaOrigin[0] != '\0') {
    http.addHeader("X-Aia-Origin", d->aiaOrigin);
  }
  int code = http.POST(body);

  if (code >= 200 && code < 300) {
    Serial.println("HTTP >> Batch enviado: " + String(d->n) + " lecturas");
  } else {
    String errMsg = (code < 0) ? http.errorToString(code) : String(code);
    Serial.println("HTTP >> Error " + String(code) + ": " + errMsg);
  }

  http.end();
  delete[] d->items;
  delete d;
}

#if defined(ESP32)
static void httpSendTask(void* param) {
  doHttpSend((HttpTaskData*)param);
  vTaskDelete(NULL);
}
#endif

YaiHttpClient::YaiHttpClient() {
  _baseUrl[0] = '\0';
  _aiaOrigin[0] = '\0';
  _bufferCount = 0;
  _lastSendTime = millis();
  _lastForzarGuardadoTime = millis();
  _forzarGuardadoIntervalMs = YAI_HTTP_FORZAR_INTERVAL_MS;
  _sending = false;
}

void YaiHttpClient::setBaseUrl(const char* url) {
  if (url && strlen(url) < sizeof(_baseUrl)) {
    strncpy(_baseUrl, url, sizeof(_baseUrl) - 1);
    _baseUrl[sizeof(_baseUrl) - 1] = '\0';
  } else {
    _baseUrl[0] = '\0';
  }
}

void YaiHttpClient::setAiaOrigin(const char* origin) {
  if (origin && strlen(origin) < sizeof(_aiaOrigin)) {
    strncpy(_aiaOrigin, origin, sizeof(_aiaOrigin) - 1);
    _aiaOrigin[sizeof(_aiaOrigin) - 1] = '\0';
  } else {
    _aiaOrigin[0] = '\0';
  }
}

void YaiHttpClient::setForzarGuardadoInterval(unsigned long ms) {
  _forzarGuardadoIntervalMs = ms;
}

void YaiHttpClient::addReading(const String& json) {
  if (!isEnabled() || _sending) return;
  if (_bufferCount >= YAI_HTTP_BUFFER_SIZE) return;
  if (json.length() == 0) return;

  _buffer[_bufferCount] = json;
  _bufferCount++;
}

bool YaiHttpClient::isEnabled() const {
  return _baseUrl[0] != '\0' && WiFi.status() == WL_CONNECTED;
}

int YaiHttpClient::getBufferCount() const {
  return _bufferCount;
}

void YaiHttpClient::loop() {
  if (!isEnabled()) return;

  unsigned long now = millis();

  // Forzar guardado periódico (spec: cada 1h, 6h, o 15-30 min)
  if (_forzarGuardadoIntervalMs > 0 && _aiaOrigin[0] != '\0' &&
      now - _lastForzarGuardadoTime >= _forzarGuardadoIntervalMs) {
    _lastForzarGuardadoTime = now;
    forzarGuardado();
  }

  if (_sending || _bufferCount == 0) return;
  if (now - _lastSendTime < YAI_HTTP_BATCH_INTERVAL_MS) return;

  flushAndSend();
}

bool YaiHttpClient::forzarGuardado() {
  if (!isEnabled() || _aiaOrigin[0] == '\0') return false;

  String url = String(_baseUrl);
  if (url.endsWith("/")) url.remove(url.length() - 1);
  url += "/monitor/api/historial/forzar-guardado";

  HTTPClient http;
#if defined(ESP8266)
  BearSSL::WiFiClientSecure client;
  client.setInsecure();
  http.begin(client, url);
#else
  http.begin(url);
#endif
  http.addHeader("X-Aia-Origin", _aiaOrigin);
  int code = http.POST("");

  bool guardado = false;
  if (code >= 200 && code < 300) {
    String payload = http.getString();
    StaticJsonDocument<128> doc;
    if (deserializeJson(doc, payload) == DeserializationError::Ok) {
      guardado = doc["guardado"] | false;
    }
    if (guardado) {
      Serial.println("HTTP >> Forzar guardado OK: " + String(doc["mensaje"] | ""));
    } else {
      Serial.println("HTTP >> Forzar guardado: " + String(doc["mensaje"] | "No guardado"));
    }
  } else {
    String errMsg = (code < 0) ? http.errorToString(code) : String(code);
    Serial.println("HTTP >> Forzar guardado error " + String(code) + ": " + errMsg);
  }
  http.end();
  return guardado;
}

void YaiHttpClient::flushAndSend() {
  if (_bufferCount == 0) return;

  _sending = true;
  int count = _bufferCount;
  String* copy = new String[count];
  for (int i = 0; i < count; i++) {
    copy[i] = _buffer[i];
  }
  _bufferCount = 0;
  _lastSendTime = millis();

  HttpTaskData* td = new HttpTaskData;
  strncpy(td->url, _baseUrl, sizeof(td->url) - 1);
  td->url[sizeof(td->url) - 1] = '\0';
  strncpy(td->aiaOrigin, _aiaOrigin, sizeof(td->aiaOrigin) - 1);
  td->aiaOrigin[sizeof(td->aiaOrigin) - 1] = '\0';
  td->items = copy;
  td->n = count;

#if defined(ESP32)
  xTaskCreate(httpSendTask, "http_send", 4096, td, 1, NULL);
#else
  doHttpSend(td);
#endif
  _sending = false;
}
