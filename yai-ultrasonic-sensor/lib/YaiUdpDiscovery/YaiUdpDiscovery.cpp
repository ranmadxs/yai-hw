#include "YaiUdpDiscovery.h"

extern bool udpDiscoveryLogsEnabled;

const char* YaiUdpDiscovery::DISCOVER_MSG = "AIA-DISCOVER";
const unsigned long YaiUdpDiscovery::SUBSCRIBER_TIMEOUT_MS = 300000;  // 5 min
const unsigned long YaiUdpDiscovery::BROADCAST_INTERVAL_MS = 2000;   // 2 seg (ESP8266 no recibe broadcast en modo AP)

YaiUdpDiscovery::YaiUdpDiscovery() : _initialized(false), _hasSubscriber(false), _onCommand(nullptr), _lastBroadcastTime(0) {
  _deviceId[0] = '\0';
  _channelId[0] = '\0';
  _version[0] = '\0';
  _mqttTopicIn[0] = '\0';
  _mqttTopicOut[0] = '\0';
  _name[0] = '\0';
  _type[0] = '\0';
}

void YaiUdpDiscovery::begin(const char* deviceId, const char* channelId, const char* version,
                           const char* mqttTopicIn, const char* mqttTopicOut,
                           const char* name, const char* type) {
  if (deviceId) strncpy(_deviceId, deviceId, sizeof(_deviceId) - 1);
  _deviceId[sizeof(_deviceId) - 1] = '\0';
  if (channelId) strncpy(_channelId, channelId, sizeof(_channelId) - 1);
  _channelId[sizeof(_channelId) - 1] = '\0';
  if (version) strncpy(_version, version, sizeof(_version) - 1);
  _version[sizeof(_version) - 1] = '\0';
  if (mqttTopicIn) strncpy(_mqttTopicIn, mqttTopicIn, sizeof(_mqttTopicIn) - 1);
  _mqttTopicIn[sizeof(_mqttTopicIn) - 1] = '\0';
  if (mqttTopicOut) strncpy(_mqttTopicOut, mqttTopicOut, sizeof(_mqttTopicOut) - 1);
  _mqttTopicOut[sizeof(_mqttTopicOut) - 1] = '\0';
  if (name) {
    strncpy(_name, name, sizeof(_name) - 1);
    _name[sizeof(_name) - 1] = '\0';
  } else {
    _name[0] = '\0';
  }
  if (type) strncpy(_type, type, sizeof(_type) - 1);
  _type[sizeof(_type) - 1] = '\0';

  _udp.begin(DISCOVERY_PORT);
  _initialized = true;
  if (udpDiscoveryLogsEnabled) Serial.println("UDP Discovery >> listening on port " + String(DISCOVERY_PORT));
}

void YaiUdpDiscovery::loop(const String& ipSta, const String& ipAp) {
  if (!_initialized) return;

  // Heartbeat cada 60s (solo si no hay subscriber, para no saturar serial)
  static unsigned long lastHeartbeat = 0;
  unsigned long now = millis();
  if (!_hasSubscriber && (now - lastHeartbeat >= 60000)) {
    lastHeartbeat = now;
    if (udpDiscoveryLogsEnabled) Serial.println("UDP Discovery >> alive, waiting for AIA-DISCOVER, ipAp=" + ipAp);
  }

  // Broadcast proactivo: ESP8266 no recibe bien broadcast cuando es AP. Enviamos cada 2s a 192.168.50.255
  if (now - _lastBroadcastTime >= BROADCAST_INTERVAL_MS) {
    _lastBroadcastTime = now;
    _broadcastDiscovery(ipAp);
  }

  int packetSize = _udp.parsePacket();
  if (packetSize <= 0) return;

  if (_hasSubscriber && (millis() - _subscriberLastSeen > SUBSCRIBER_TIMEOUT_MS)) {
    _hasSubscriber = false;
    if (udpDiscoveryLogsEnabled) Serial.println("UDP Discovery >> subscriber timeout");
  }

  char buf[256];
  int len = _udp.read(buf, sizeof(buf) - 1);
  buf[len] = '\0';

  IPAddress remoteIP = _udp.remoteIP();
  uint16_t remotePort = _udp.remotePort();
  int previewLen = (len > 60) ? 60 : len;
  if (udpDiscoveryLogsEnabled) Serial.println("UDP >> received " + String(len) + " bytes from " + remoteIP.toString() + ":" + String(remotePort) + " [" + String(buf).substring(0, previewLen) + "]");

  if (strstr(buf, DISCOVER_MSG) != NULL) {
    _subscriberIP = remoteIP;
    _subscriberPort = remotePort;
    _subscriberLastSeen = millis();
    _hasSubscriber = true;
    if (udpDiscoveryLogsEnabled) Serial.println("UDP Discovery >> subscriber registered: " + remoteIP.toString() + ":" + String(remotePort));
    // Si el cliente está en la red del AP (192.168.50.x), responder con IP del AP para que pueda conectarse
    bool remoteOnAp = (remoteIP[0] == 192 && remoteIP[1] == 168 && remoteIP[2] == 50);
    String ipToSend = remoteOnAp ? ipAp : (ipSta.length() > 0 ? ipSta : ipAp);
    _sendResponse(remoteIP, remotePort, ipToSend);
    return;
  }

  _handleCommand(buf);
}

void YaiUdpDiscovery::_sendResponse(const IPAddress& remoteIP, uint16_t remotePort, const String& ip) {
  StaticJsonDocument<384> doc;
  // name = título (Estanque Costa), id = subtítulo (v1.2.1-YUS-COSTA) — app usa name=title, id=subtitle
  doc["name"] = (_name[0] != '\0') ? _name : _deviceId;
  String idVal = (_name[0] != '\0') ? String("v") + _version : String(_deviceId);
  doc["id"] = idVal;
  doc["device_id"] = _deviceId;
  doc["channel_id"] = _channelId;
  doc["ip"] = ip;
  doc["version"] = _version;
  doc["mqtt_topic_in"] = _mqttTopicIn;
  doc["mqtt_topic_out"] = _mqttTopicOut;
  doc["type"] = _type;

  String json;
  serializeJson(doc, json);

  _udp.beginPacket(remoteIP, remotePort);
  _udp.write((const uint8_t*)json.c_str(), json.length());
  _udp.endPacket();

  if (udpDiscoveryLogsEnabled) Serial.println("UDP Discovery >> responded to " + remoteIP.toString() + ":" + String(remotePort));
}

void YaiUdpDiscovery::_broadcastDiscovery(const String& ipAp) {
  if (ipAp.length() == 0) return;

  StaticJsonDocument<384> doc;
  // name = título (Estanque Costa), id = subtítulo (v1.2.1-YUS-COSTA) — app usa name=title, id=subtitle
  doc["name"] = (_name[0] != '\0') ? _name : _deviceId;
  String idVal = (_name[0] != '\0') ? String("v") + _version : String(_deviceId);
  doc["id"] = idVal;
  doc["device_id"] = _deviceId;
  doc["channel_id"] = _channelId;
  doc["ip"] = ipAp;
  doc["version"] = _version;
  doc["mqtt_topic_in"] = _mqttTopicIn;
  doc["mqtt_topic_out"] = _mqttTopicOut;
  doc["type"] = _type;

  String json;
  serializeJson(doc, json);

  // Broadcast a subnet (192.168.50.255) y a limited broadcast (255.255.255.255)
  IPAddress subnetBroadcast(192, 168, 50, 255);
  IPAddress limitedBroadcast(255, 255, 255, 255);

  _udp.beginPacket(subnetBroadcast, DISCOVERY_PORT);
  _udp.write((const uint8_t*)json.c_str(), json.length());
  _udp.endPacket();

  _udp.beginPacket(limitedBroadcast, DISCOVERY_PORT);
  _udp.write((const uint8_t*)json.c_str(), json.length());
  _udp.endPacket();
}

void YaiUdpDiscovery::_handleCommand(const char* buf) {
  StaticJsonDocument<256> doc;
  DeserializationError err = deserializeJson(doc, buf);
  if (err) return;

  const char* topic = doc["topic"];
  if (!topic || strcmp(topic, _mqttTopicIn) != 0) return;

  const char* payload = doc["payload"];
  if (!payload || _onCommand == nullptr) return;

  _onCommand(payload);
  if (udpDiscoveryLogsEnabled) Serial.println("UDP >> command: " + String(payload));
}

void YaiUdpDiscovery::sendToTopic(const char* topic, const String& payload) {
  if (!_hasSubscriber || payload.length() == 0) return;

  String msg = "{\"topic\":\"" + String(topic) + "\",\"payload\":" + payload + "}";

  _udp.beginPacket(_subscriberIP, _subscriberPort);
  _udp.write((const uint8_t*)msg.c_str(), msg.length());
  _udp.endPacket();
  if (udpDiscoveryLogsEnabled) Serial.println("UDP >> sent " + String(topic) + " to " + _subscriberIP.toString() + ":" + String(_subscriberPort));
}
