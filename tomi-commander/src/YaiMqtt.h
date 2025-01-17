#ifndef YaiMqtt_h
#define YaiMqtt_h
#include <Arduino.h>
#include <PubSubClient.h>
#include "YaiCommons.h"

extern Metrics metrics;

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
  LOG_DEBUG(logger, "Message arrived [" + String(topic) + "] " + msgPayload);
  metrics.sendCountMetric("yai.mqtt.message.in.count",1);

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
      //metrics.sendCountMetric("yai.mqtt.message.ok.count",1);
      commandFactoryExecute(yaiCommand);

      int pins[8];
      getPinsArray(yaiCommand, pins);

      for (int i = 0; i < 8; i++) {
        if (pins[i] > 0) {
          String keyName = String(pins[i]);
          metrics.sendCountMetric("yai.mqtt.message.pin." + keyName + ".count", 1);
        }
      }


    } else {
      metrics.sendCountMetric("yai.mqtt.message.error.count",1);
      LOG_ERROR(logger, "Command not found");
    }
  } else {
    metrics.sendCountMetric("yai.mqtt.message.error.malformed",1);
    LOG_ERROR(logger, "MALFORMED COMMAND");
  }
  //Serial.println();
}

void reconnect() {
  // Verificar si ha pasado el tiempo definido desde el último intento de reconexión
  if (!clientMqtt.connected() && (millis() - LAST_RECONNECT_ATTEMPT > RECONNECT_INTERVAL)) {
    LAST_RECONNECT_ATTEMPT = millis(); // Actualizar el último intento de reconexión
    LOG_INFO(logger, "Attempting MQTT connection...");
    // Crear un ID de cliente aleatorio
    String clientId = String(YAI_UID_NAME) + " [NodeMCU-ESP32]";
    clientId += String(random(0xffff), HEX);
    // Intentar conectar
    if (clientMqtt.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD)) {
      metrics.sendCountMetric("yai.mqtt.status.ok.count",1);
      LOG_INFO(logger, "connected");
      // Una vez conectado, publicar un mensaje...
      clientMqtt.publish(MQTT_TOPIC_OUT, ("hello world "+clientId).c_str()); //outTopic
      // ... y volver a suscribirse
      clientMqtt.subscribe(MQTT_TOPIC_IN); //inYaiTopic
    } else {
      metrics.sendCountMetric("yai.mqtt.status.error.count",1);
      String errorMsg = "failed, rc=" + String(clientMqtt.state()) +  " try again in " + String(RECONNECT_INTERVAL / 1000) + "[s]";
      LOG_ERROR(logger, errorMsg);
    }
  }
}



#endif