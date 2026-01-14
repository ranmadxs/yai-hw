#ifndef YaiMqtt_h
#define YaiMqtt_h
#include <Arduino.h>
#include <PubSubClient.h>

// External declarations
extern const String DEVICE_ID;

//https://www.hivemq.com/public-mqtt-broker/
/*
mosquitto_sub -h localhost -p 1883 -t test/topic
mosquitto_pub -d -h localhost -p 1883 -t "test/topic" -m "Hola Mundo"

*/

void callbackMqtt(char* topic, byte* payload, unsigned int length) {
  String msgPayload = "";
  bool existCmd = false;
  for (int i = 0; i < length; i++) {
    msgPayload = msgPayload + (char)payload[i];
  }
  msgPayload.toUpperCase();
  Serial.println("Message arrived [" + String(topic) + "] " + msgPayload);

  if(msgPayload.length() > 10 && msgPayload.indexOf(",") > 0) {
    Serial.println("MQTT Command received");
    YaiCommand yaiCommand;
    yaiCommand.type = "MQTT";
    yaiCommand.message = msgPayload;
    yaiUtil.string2YaiCommand(yaiCommand);
    yaiCommand.execute = EXECUTE_CMD;
    // Commands will be validated in commandFactoryExecute
    existCmd = false;
    if (existCmd) {
      commandFactoryExecute(yaiCommand);
    } else {
      Serial.println("Command not found");
    }
  } else {
    Serial.println("MALFORMED COMMAND");
  }
  //Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  while (!clientMqtt.connected()) {
    Serial.print("Attempting MQTT connection...");
    Serial.println(" Topics: IN=" + String(MQTT_TOPIC_IN) + " OUT=" + String(MQTT_TOPIC_OUT));
    // Create a random client ID
    String clientId = String(YAI_UID_NAME) + " [NodeMCU-ESP32]";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (clientMqtt.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD)) {
      Serial.println("MQTT connected");
      // Once connected, publish an announcement...
      clientMqtt.publish(MQTT_TOPIC_OUT, ("hello world "+clientId).c_str()); //outTopic

      // Publicar información completa de configuración MQTT
      String deviceTopic = "yai-mqtt/" + DEVICE_ID + "/out";
      String mqttInfo = "MQTT_CONFIG: Server=" + String(MQTT_SERVER) + ":" + String(MQTT_PORT) +
                       ", User=" + String(MQTT_USER) +
                       ", Device=" + DEVICE_ID +
                       ", Listening=" + String(MQTT_TOPIC_IN) +
                       ", Responding=" + String(MQTT_TOPIC_OUT) + " and " + deviceTopic +
                       ", Commands=ON,<interval>,0,0,0,0,0,0 | OFF,0,0,0,0,0,0,0";
      clientMqtt.publish(MQTT_TOPIC_OUT, mqttInfo.c_str());

      // Mostrar también por serial
      Serial.println("MQTT Configuration published:");
      Serial.println("- Server: " + String(MQTT_SERVER) + ":" + String(MQTT_PORT));
      Serial.println("- User: " + String(MQTT_USER));
      Serial.println("- Device: " + DEVICE_ID);
      Serial.println("- Listening on: " + String(MQTT_TOPIC_IN));
      Serial.println("- Responding on: " + String(MQTT_TOPIC_OUT) + " and " + deviceTopic);

      // ... and resubscribe
      clientMqtt.subscribe(MQTT_TOPIC_IN); //inYaiTopic
    } else {
      Serial.print("MQTT connection failed, rc=");
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
        String clientId = "ESP32Client-";
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

