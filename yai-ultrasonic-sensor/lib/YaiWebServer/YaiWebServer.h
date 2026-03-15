#ifndef YaiWebServer_h
#define YaiWebServer_h

#include <Arduino.h>

#if defined(ESP8266)
#include <ESP8266WebServer.h>
#elif defined(ESP32)
#include <WebServer.h>
#endif

/** Callback para restaurar la conexión principal del NodeMCU tras el test de credenciales */
typedef void (*WifiRestoreCallback)();
/** Callback para guardar ssid/password en flash cuando el test pasa */
typedef void (*WifiSaveCallback)(const char* ssid, const char* password);

/**
 * Servidor HTTP para configuración WiFi.
 * - GET /api/wifi: lista redes visibles y la conectada (cache, respuesta inmediata)
 * - POST /api/wifi: recibe {ssid, password}, intenta conectar, log si OK
 */
class YaiWebServer {
public:
  YaiWebServer();

  void begin();
  void loop();
  void setWifiRestoreCallback(WifiRestoreCallback cb) { _wifiRestoreCb = cb; }
  void setWifiSaveCallback(WifiSaveCallback cb) { _wifiSaveCb = cb; }

private:
#if defined(ESP8266)
  ESP8266WebServer _server;
#else
  WebServer _server;
#endif

  String _cachedWifiJson;
  unsigned long _cacheValidUntil;
  bool _scanInProgress;
  WifiRestoreCallback _wifiRestoreCb;
  WifiSaveCallback _wifiSaveCb;

  bool _testWifiCredentials(const char* ssid, const char* password);
  void _handleGetWifi();
  void _handlePostWifi();
  void _handleNotFound();
  void _startAsyncScan();
  void _updateScanCache();
  String _buildWifiJson(int n);
};

#endif
