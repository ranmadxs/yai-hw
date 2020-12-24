#ifndef YaiMqtt_h
#define YaiMqtt_h
#include <Arduino.h>
#include <PubSubClient.h>
#include "YaiContext.h"

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived XD [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

class YaiMqtt {
  public:

    YaiMqtt(PubSubClient _clientMqtt){
      clientMqtt = _clientMqtt;

    };

    void reconnect() {
      // Loop until we're reconnected
      while (!clientMqtt.connected()) {
        Serial.print("Attempting MQTT connection...");
        // Create a random client ID
        String clientId = "ESP8266Client-";
        clientId += String(random(0xffff), HEX);
        // Attempt to connect
        if (clientMqtt.connect(clientId.c_str())) {
          Serial.println("connected");
          // Once connected, publish an announcement...
          clientMqtt.publish("outTopic", "hello world");
          // ... and resubscribe
          clientMqtt.subscribe("inYaiTopic");
        } else {
          Serial.print("failed, rc=");
          Serial.print(clientMqtt.state());
          Serial.println(" try again in 5 seconds");
          // Wait 5 seconds before retrying
          delay(5000);
        }
      }
    }
  private:
    PubSubClient clientMqtt;
};

#endif