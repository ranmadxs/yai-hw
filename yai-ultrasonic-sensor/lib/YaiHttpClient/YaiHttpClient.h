#ifndef YaiHttpClient_h
#define YaiHttpClient_h

#include <Arduino.h>
#if defined(ESP32)
#include <HTTPClient.h>
#include <WiFiClient.h>
#elif defined(ESP8266)
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#endif

#define YAI_HTTP_BUFFER_SIZE 60
#define YAI_HTTP_BATCH_INTERVAL_MS 60000   // 1 minuto
#define YAI_HTTP_FORZAR_INTERVAL_MS 10000  // 10 segundos (temporal, spec recomienda 1h/6h/15-30min)

/**
 * Cliente HTTP que acumula lecturas y envía en batch cada 1 minuto.
 * También llama al endpoint forzar-guardado periódicamente (cada hora por defecto).
 * No bloquea el loop principal: el envío batch se hace en un task de FreeRTOS.
 */
class YaiHttpClient {
public:
  YaiHttpClient();

  /** Configura la URL base (ej: "https://tomicolector.cl") */
  void setBaseUrl(const char* url);

  /** Configura el identificador para header X-Aia-Origin (ej: "YUS_028costa" o "YUS-1.0.5@1A2B3C4D") */
  void setAiaOrigin(const char* origin);

  /** Agrega una lectura JSON al buffer. Se envía en el próximo batch. */
  void addReading(const String& json);

  /** Debe llamarse desde loop(). Revisa si pasó 1 min y envía en background; cada hora llama forzarGuardado. */
  void loop();

  /** Retorna true si hay URL configurada y WiFi conectado */
  bool isEnabled() const;

  /** Cantidad de lecturas en buffer */
  int getBufferCount() const;

  /**
   * Llama al endpoint forzar-guardado (POST sin body, header X-Aia-Origin).
   * Spec: https://tomicolector.cl/monitor/api/historial/forzar-guardado
   * Retorna true si guardado OK, false si no guardado o error.
   */
  bool forzarGuardado();

  /** Intervalo para forzar guardado automático (ms). 0 = deshabilitado. Default: 1 hora. */
  void setForzarGuardadoInterval(unsigned long ms);

private:
  void flushAndSend();

  char _baseUrl[256];
  char _aiaOrigin[64];
  String _buffer[YAI_HTTP_BUFFER_SIZE];
  int _bufferCount;
  unsigned long _lastSendTime;
  unsigned long _lastForzarGuardadoTime;
  unsigned long _forzarGuardadoIntervalMs;
  bool _sending;
};

#endif
