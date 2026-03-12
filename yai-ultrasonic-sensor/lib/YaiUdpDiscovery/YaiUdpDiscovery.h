#ifndef YaiUdpDiscovery_h
#define YaiUdpDiscovery_h

#include <Arduino.h>
#include <ArduinoJson.h>

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#elif defined(ESP32)
#include <WiFi.h>
#include <WiFiUdp.h>
#else
#error "Plataforma no soportada. Solo ESP8266 y ESP32."
#endif

/** Callback para comandos recibidos por UDP (topic yai-mqtt/in) */
typedef void (*YaiUdpCommandCallback)(const char* payload);

/**
 * UDP Discovery para app Android AIA Agent.
 * Escucha en puerto 9999 el mensaje "AIA-DISCOVER" y responde con JSON.
 * También envía lecturas por topic yai-mqtt/out y recibe comandos por yai-mqtt/in.
 * Spec: specs/UDP-DISCOVERY-MICROCONTROLLER-SPEC.md
 */
class YaiUdpDiscovery {
public:
  YaiUdpDiscovery();
  void begin(const char* deviceId, const char* channelId, const char* version,
             const char* mqttTopicIn, const char* mqttTopicOut,
             const char* name = nullptr, const char* type = "estanque");
  /** ipSta: IP cuando conectado al router. ipAp: IP del AP (192.168.50.1) para clientes conectados al ESP */
  void loop(const String& ipSta, const String& ipAp = "192.168.50.1");

  /** Envía un mensaje al subscriber por el topic indicado (ej: yai-mqtt/out para lecturas) */
  void sendToTopic(const char* topic, const String& payload);

  /** true si hay app conectada (recibió AIA-DISCOVER) y no ha expirado */
  bool hasSubscriber() const { return _hasSubscriber; }

  /** Callback cuando se recibe un comando por topic yai-mqtt/in */
  void setOnCommandCallback(YaiUdpCommandCallback cb) { _onCommand = cb; }

private:
  static const int DISCOVERY_PORT = 9999;
  static const char* DISCOVER_MSG;
  static const unsigned long SUBSCRIBER_TIMEOUT_MS;
  static const unsigned long BROADCAST_INTERVAL_MS;  // Broadcast proactivo (ESP8266 no recibe bien en modo AP)

  WiFiUDP _udp;
  IPAddress _subscriberIP;
  uint16_t _subscriberPort;
  unsigned long _subscriberLastSeen;
  unsigned long _lastBroadcastTime;
  bool _hasSubscriber;
  YaiUdpCommandCallback _onCommand;
  char _deviceId[64];
  char _channelId[32];
  char _version[32];
  char _mqttTopicIn[96];
  char _mqttTopicOut[96];
  char _name[64];
  char _type[24];
  bool _initialized;

  void _sendResponse(const IPAddress& remoteIP, uint16_t remotePort, const String& ip);
  void _broadcastDiscovery(const String& ipAp);
  void _handleCommand(const char* buf);
};

#endif
