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
  bool* sendingFlag;  // para resetear _sending si se aborta
};

struct ForzarGuardadoTaskData {
  char url[256];
  char aiaOrigin[64];
  bool* inProgressFlag;
};

static void doHttpSend(HttpTaskData* d) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("HTTP >> Batch omitido: WiFi no conectado (ej. tras test /api/wifi)");
    if (d->sendingFlag) *d->sendingFlag = false;
    delete[] d->items;
    delete d;
    return;
  }

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
  http.setTimeout(90);  // 90 segundos (read timeout)
#else
  http.begin(endpoint);
  http.setTimeout(60000);  // 60 segundos en ms
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
  if (d->sendingFlag) *d->sendingFlag = false;
  delete[] d->items;
  delete d;
}

#if defined(ESP32)
static void httpSendTask(void* param) {
  doHttpSend((HttpTaskData*)param);
  vTaskDelete(NULL);
}

static void forzarGuardadoTask(void* param) {
  ForzarGuardadoTaskData* d = (ForzarGuardadoTaskData*)param;
  const uint16_t TIMEOUT_MS = 65535;
  const int MAX_RETRIES = 2;

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("HTTP >> Forzar guardado omitido: WiFi no conectado");
    if (d->inProgressFlag) *d->inProgressFlag = false;
    delete d;
    vTaskDelete(NULL);
    return;
  }

  for (int attempt = 0; attempt <= MAX_RETRIES; attempt++) {
    HTTPClient http;
    http.begin(d->url);
    http.setTimeout(TIMEOUT_MS);
    http.setConnectTimeout(300000);
    http.addHeader("X-Aia-Origin", d->aiaOrigin);
    int code = http.POST("");

    if (code >= 200 && code < 300) {
      String payload = http.getString();
      StaticJsonDocument<128> doc;
      bool guardado = false;
      if (deserializeJson(doc, payload) == DeserializationError::Ok) {
        guardado = doc["guardado"] | false;
      }
      Serial.println("HTTP >> Forzar guardado " + String(guardado ? "OK" : "no guardado") + ": " + String(doc["mensaje"] | ""));
      http.end();
      break;
    }
    String errMsg = (code < 0) ? http.errorToString(code) : String(code);
    http.end();
    bool esTimeout = (code == -1 || code == -11);
    if (esTimeout && attempt < MAX_RETRIES) {
      Serial.println("HTTP >> Forzar guardado timeout, reintento " + String(attempt + 2) + "/" + String(MAX_RETRIES + 1) + "...");
      vTaskDelay(pdMS_TO_TICKS(10000));
    } else {
      Serial.println("HTTP >> Forzar guardado error " + String(code) + ": " + errMsg);
      break;
    }
  }
  if (d->inProgressFlag) *d->inProgressFlag = false;
  delete d;
  vTaskDelete(NULL);
}
#endif

YaiHttpClient::YaiHttpClient() {
  _baseUrl[0] = '\0';
  _aiaOrigin[0] = '\0';
  _forzarGuardadoOrigin[0] = '\0';
  _bufferCount = 0;
  _lastSendTime = millis();
  _lastForzarGuardadoTime = millis();
  _forzarGuardadoIntervalMs = YAI_HTTP_FORZAR_INTERVAL_MS;
  _sending = false;
#if defined(ESP32)
  _forzarGuardadoInProgress = false;
#endif
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

void YaiHttpClient::setForzarGuardadoOrigin(const char* origin) {
  if (origin && strlen(origin) < sizeof(_forzarGuardadoOrigin)) {
    strncpy(_forzarGuardadoOrigin, origin, sizeof(_forzarGuardadoOrigin) - 1);
    _forzarGuardadoOrigin[sizeof(_forzarGuardadoOrigin) - 1] = '\0';
  } else {
    _forzarGuardadoOrigin[0] = '\0';
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

  // Forzar guardado periódico. En ESP32 corre en task para no bloquear sensor/WebServer.
  const char* fgOrigin = (_forzarGuardadoOrigin[0] != '\0') ? _forzarGuardadoOrigin : _aiaOrigin;
  if (_forzarGuardadoIntervalMs > 0 && fgOrigin[0] != '\0' &&
      now - _lastForzarGuardadoTime >= _forzarGuardadoIntervalMs) {
    _lastForzarGuardadoTime = now;
#if defined(ESP32)
    if (!_forzarGuardadoInProgress) {
      _forzarGuardadoInProgress = true;
      ForzarGuardadoTaskData* td = new ForzarGuardadoTaskData;
      String url = String(_baseUrl);
      if (url.endsWith("/")) url.remove(url.length() - 1);
      url += "/monitor/api/historial/forzar-guardado";
      strncpy(td->url, url.c_str(), sizeof(td->url) - 1);
      td->url[sizeof(td->url) - 1] = '\0';
      strncpy(td->aiaOrigin, fgOrigin, sizeof(td->aiaOrigin) - 1);
      td->inProgressFlag = &_forzarGuardadoInProgress;
      xTaskCreate(forzarGuardadoTask, "forzar_guardado", 12288, td, 1, NULL);
    }
#else
    forzarGuardado();
#endif
  }

  if (_sending || _bufferCount == 0) return;
  if (now - _lastSendTime < YAI_HTTP_BATCH_INTERVAL_MS) return;

  flushAndSend();
}

bool YaiHttpClient::forzarGuardado() {
  const char* fgOrigin = (_forzarGuardadoOrigin[0] != '\0') ? _forzarGuardadoOrigin : _aiaOrigin;
  if (!isEnabled() || fgOrigin[0] == '\0') return false;

  String url = String(_baseUrl);
  if (url.endsWith("/")) url.remove(url.length() - 1);
  url += "/monitor/api/historial/forzar-guardado";

  const int MAX_RETRIES = 2;
  // ESP8266 HTTPClient::setTimeout usa uint16_t (max 65535 ms = 65 s). ESP32 idem.
  const uint16_t TIMEOUT_MS = 65535;  // máx permitido por API

  for (int attempt = 0; attempt <= MAX_RETRIES; attempt++) {
    HTTPClient http;
#if defined(ESP8266)
    BearSSL::WiFiClientSecure client;
    client.setInsecure();
    client.setTimeout(TIMEOUT_MS);  // ms: conexión TLS + lectura (NodeMCU más lento)
    http.begin(client, url);
    http.setTimeout(TIMEOUT_MS);
#else
    http.begin(url);
    http.setTimeout(TIMEOUT_MS);
    http.setConnectTimeout(300000);  // 5 min para cold start (Railway)
#endif
    http.addHeader("X-Aia-Origin", fgOrigin);
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
      http.end();
      return guardado;
    }

    String errMsg = (code < 0) ? http.errorToString(code) : String(code);
    http.end();

    // -1 = connection timeout, -11 = read timeout
    bool esTimeout = (code == -1 || code == -11);
    if (esTimeout && attempt < MAX_RETRIES) {
      Serial.println("HTTP >> Forzar guardado timeout (" + String(code) + "), reintento " + String(attempt + 2) + "/" + String(MAX_RETRIES + 1) + " en 10s...");
      delay(10000);  // 10s entre reintentos (Railway cold start)
    } else {
      Serial.println("HTTP >> Forzar guardado error " + String(code) + ": " + errMsg);
      return false;
    }
  }
  return false;
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
  td->sendingFlag = &_sending;

#if defined(ESP32)
  // HTTPS/TLS requiere ~8-12KB stack. 4096 causa stack overflow -> Guru Meditation en ESP32-S3
  xTaskCreate(httpSendTask, "http_send", 12288, td, 1, NULL);
#else
  doHttpSend(td);
#endif
  _sending = false;
}
