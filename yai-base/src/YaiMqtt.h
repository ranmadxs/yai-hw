#ifndef YaiMqtt_h
#define YaiMqtt_h
#include <Arduino.h>
#include <PubSubClient.h>
#include "YaiContext.h"
//https://www.hivemq.com/public-mqtt-broker/
void callback(char* topic, byte* payload, unsigned int length) {
  String msgPayload = "";
  bool existCmd = false;
  for (int i = 0; i < length; i++) {
    msgPayload = msgPayload + (char)payload[i];
  }
  msgPayload.toUpperCase();
  logger.debug("Message arrived [" + String(topic) + "] " + msgPayload);

  if(msgPayload.length() > 10 && msgPayload.indexOf("\"CODE\":\"CMD\"") > 0) {
    logger.debug("WIIIIIIIIIII CMD");
    YaiCommand yaiCommand;
    yaiCommand.type = "MQTT";
    yaiCommand.execute = EXECUTE_CMD;
    if(msgPayload.indexOf("ENCENDER BOMBA") > 0 || msgPayload.indexOf("INICIAR BOMBA") > 0) {
      //logger.debug("ON BOMBA");
      yaiCommand.command = "ON";
      existCmd = true;
    }
    if(msgPayload.indexOf("APAGAR BOMBA") > 0 || msgPayload.indexOf("DETENER BOMBA") > 0) {
      //logger.debug("OFF BOMBA");
      yaiCommand.command = "OFF";
      existCmd = true;
    }
    if (existCmd) {
      //logger.info(yaiCommand.toString());
      commandFactoryExecute(yaiCommand);
    } else {
      logger.error("Command not found");
    }
  } else {
    logger.error("MALFORMED JSON COMMAND");
  }
  //Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  while (!clientMqtt.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = String(YAI_UID_NAME) + " [NodeMCU-ESP8266]";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (clientMqtt.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      clientMqtt.publish(MQTT_TOPIC_OUT, ("hello world "+clientId).c_str()); //outTopic
      // ... and resubscribe
      clientMqtt.subscribe(MQTT_TOPIC_ALL); //inYaiTopic
    } else {
      Serial.print("failed, rc=");
      Serial.print(clientMqtt.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
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
          Serial.println("http://www.hivemq.com/demos/websocket-client/");
          // Wait 5 seconds before retrying
          delay(5000);
        }
      }
    }
  private:
    PubSubClient clientMqtt;
};

#endif