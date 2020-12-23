#ifndef YaiPubSub_h
#define YaiPubSub_h

#include <WiFiClient.h>
#include <PubSubClient.h>
#define YAI_CALLBACK void (*callback)(char*, uint8_t*, unsigned int)

class YaiPubSub {
  public:         
   
    YaiPubSub(WiFiClient wifiClient, YAI_CALLBACK) {
      pubSubClient = new PubSubClient(wifiClient);
      callback = YAI_CALLBACK;
    }

    PubSubClient* pubSubClient;

    void start() {
      pubSubClient->setServer(mqtt_server, mqtt_port);
      pubSubClient->setCallback(callback);
    }

    void reconnect() {
      // Loop until we're reconnected
      while (!pubSubClient->connected()) {
        Serial.print("Attempting MQTT connection...");
        // Create a random client ID
        String clientId = "ESP8266Client-";
        clientId += String(random(0xffff), HEX);
        // Attempt to connect
        if (pubSubClient->connect(clientId.c_str())) {
          Serial.println("connected");
          // Once connected, publish an announcement...
          pubSubClient->publish("outTopic", "hello world");
          // ... and resubscribe
          pubSubClient->subscribe("inYaiTopic");
        } else {
          pubSubClient->print("failed, rc=");
          pubSubClient->print(pubSubClient->state());
          pubSubClient->println(" try again in 5 seconds");
          // Wait 5 seconds before retrying
          delay(5000);
        }
      }
    }

  private:
    void (*callback)(char*, uint8_t*, unsigned int);
    const char* mqtt_server = "broker.hivemq.com";
    const int mqtt_port = 1883;
    //const char* mqtt_server = "192.168.1.40";
    const char* mqtt_user = "test";
    const char* mqtt_password = "test";

};

#endif