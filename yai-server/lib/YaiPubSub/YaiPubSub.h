#ifndef YaiPubSub_h
#define YaiPubSub_h

#include <WiFiClient.h>
#include <PubSubClient.h>
#define YAI_CALLBACK void (*callback)(char*, uint8_t*, unsigned int)

class YaiPubSub {
  public:         
   
    YaiPubSub(WiFiClient wifiClient, YAI_CALLBACK) {
      pubSubClient = new PubSubClient(wifiClient);
    }

    PubSubClient* pubSubClient;

    YaiPubSub& setCallback(MQTT_CALLBACK_SIGNATURE) {
      this->callback = callback;
      return *this;
    }

    void start() {
      pubSubClient->setServer(mqtt_server, mqtt_port);
      pubSubClient->setCallback(this->callback);      
    }

    void reconnect() {
      if (!client.connected()) {
        client.connect("ESP8266Client");    
        client.subscribe("inYaiTopic");
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