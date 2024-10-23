#ifndef YaiMqtt_h
#define YaiMqtt_h
#include <Arduino.h>
#include <PubSubClient.h>

//https://www.hivemq.com/public-mqtt-broker/
/*
mosquitto_sub -h localhost -p 1883 -t test/topic
mosquitto_pub -d -h localhost -p 1883 -t "test/topic" -m "Hola Mundo"

*/
const unsigned long RECONNECT_INTERVAL = 20000; // Intervalo de tiempo de 5 segundos
unsigned long LAST_RECONNECT_ATTEMPT = RECONNECT_INTERVAL/4; // Variable para almacenar el último intento de reconexión


void callbackMqtt(char* topic, byte* payload, unsigned int length) {
  String msgPayload = "";
  bool existCmd = false;
  for (int i = 0; i < length; i++) {
    msgPayload = msgPayload + (char)payload[i];
  }
  msgPayload.toUpperCase();
  Serial.println("Message arrived [" + String(topic) + "] " + msgPayload);

  if(msgPayload.length() > 10 && msgPayload.indexOf(",") > 0) {
    Serial.println("WIIIIIIIIIII CMD");
    YaiCommand yaiCommand;
    yaiCommand.type = "MQTT";
    yaiCommand.message = msgPayload;
    yaiUtil.string2YaiCommand(yaiCommand);
    yaiCommand.execute = EXECUTE_CMD;
    if(yaiCommand.command == "ON" || yaiCommand.command == "OFF") {
      existCmd = true;
    }
    if (existCmd) {
      commandFactoryExecute(yaiCommand);
    } else {
      LOG_ERROR(logger, "Command not found");
    }
  } else {
    LOG_ERROR(logger, "MALFORMED COMMAND");
  }
  //Serial.println();
}

void reconnect() {
  // Verificar si ha pasado el tiempo definido desde el último intento de reconexión
  if (!clientMqtt.connected() && (millis() - LAST_RECONNECT_ATTEMPT > RECONNECT_INTERVAL)) {
    LAST_RECONNECT_ATTEMPT = millis(); // Actualizar el último intento de reconexión
    Serial.print("Attempting MQTT connection...");
    // Crear un ID de cliente aleatorio
    String clientId = String(YAI_UID_NAME) + " [NodeMCU-ESP32]";
    clientId += String(random(0xffff), HEX);
    // Intentar conectar
    if (clientMqtt.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD)) {
      Serial.println("connected");
      // Una vez conectado, publicar un mensaje...
      clientMqtt.publish(MQTT_TOPIC_OUT, ("hello world "+clientId).c_str()); //outTopic
      // ... y volver a suscribirse
      clientMqtt.subscribe(MQTT_TOPIC_IN); //inYaiTopic
    } else {
      Serial.print("failed, rc=");
      Serial.print(clientMqtt.state());
      Serial.print(" try again in ");
      Serial.print(RECONNECT_INTERVAL / 1000); // Imprime el intervalo en segundos
      Serial.println(" seconds");
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
        String clientId = MQTT_CLIENT_ID;
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