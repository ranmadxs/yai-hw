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

  // Determinar si el mensaje viene del canal general o específico
  String topicStr = String(topic);
  extern const String DEVICE_MQTT_TOPIC_IN;
  bool isGeneralChannel = (topicStr == String(MQTT_TOPIC_IN));
  bool isSpecificChannel = (topicStr == DEVICE_MQTT_TOPIC_IN);

  // Comandos solo en canal GENERAL (mismo que PING): PING y HELP
  if (isGeneralChannel && (msgPayload == "PING" || msgPayload == "HELP")) {
    if (msgPayload == "PING") {
      Serial.println("PING received on general channel - sending PONG");
      extern const String DEVICE_MQTT_TOPIC_OUT;
      extern const String DEVICE_MQTT_TOPIC_IN;
      String pongResponse = "PONG," + DEVICE_ID +
                         ",IP:" + yaiWifi.getIp() +
                         ",IN:" + String(MQTT_TOPIC_IN) +
                         ",OUT:" + String(MQTT_TOPIC_OUT) +
                         ",DEVICE_IN:" + DEVICE_MQTT_TOPIC_IN +
                         ",DEVICE_OUT:" + DEVICE_MQTT_TOPIC_OUT +
                         ",SERVER:" + String(MQTT_SERVER) + ":" + String(MQTT_PORT);
      clientMqtt.publish(MQTT_TOPIC_OUT, pongResponse.c_str());
    } else {
      // HELP - respuesta estilo Slack (emojis, secciones) en el mismo canal que PONG
      Serial.println("HELP received on general channel - sending help");
      extern const String DEVICE_MQTT_TOPIC_OUT;
      extern const String DEVICE_MQTT_TOPIC_IN;
      String helpMsg = String("") +
        "📋 *YAI Ultrasonic Sensor – Ayuda*\n\n" +
        "📡 *Canal general* `yai-mqtt/in` (solo escucha aquí):\n" +
        "  🏓 `PING` → responde PONG con IP y topics\n" +
        "  ❓ `HELP` → muestra esta ayuda\n\n" +
        "⚙️ *Canal específico* `yai-mqtt/" + DEVICE_ID + "/in`:\n" +
        "  🟢 `ON,<ms>,0,0,0,0,0,0` → activar lecturas (ej: ON,2000 = cada 2s)\n" +
        "  🔴 `OFF,0,0,0,0,0,0,0` → desactivar lecturas\n\n" +
        "📈 *Lecturas* en `yai-mqtt/" + DEVICE_ID + "/out`:\n" +
        "  Formato: `DEVICE_ID,OKO|NOK,distancia_cm,timestamp`\n" +
        "  ✅ OKO = medición válida (2–400 cm)\n" +
        "  ❌ NOK = error o fuera de rango";
      clientMqtt.publish(MQTT_TOPIC_OUT, helpMsg.c_str());
    }
    return;
  }
  if (msgPayload == "PING" || msgPayload == "HELP") {
    Serial.println("PING/HELP ignored - only accepted on general channel");
    return;
  }

  // Para otros comandos (ON/OFF), solo se aceptan en el canal ESPECÍFICO
  if (isSpecificChannel) {
    if(msgPayload.length() > 10 && msgPayload.indexOf(",") > 0) {
      Serial.println("MQTT Command received on specific channel");
      YaiCommand yaiCommand;
      yaiCommand.type = "MQTT";
      yaiCommand.message = msgPayload;
      yaiUtil.string2YaiCommand(yaiCommand);
      yaiCommand.execute = EXECUTE_CMD;
      // Execute command directly
      commandFactoryExecute(yaiCommand);
    } else {
      Serial.println("MALFORMED COMMAND on specific channel");
    }
  } else if (isGeneralChannel) {
    Serial.println("Command ignored - only PING accepted on general channel");
  } else {
    Serial.println("Message from unknown channel ignored");
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
      extern const String DEVICE_MQTT_TOPIC_IN;
      extern const String DEVICE_MQTT_TOPIC_OUT;
      String mqttInfo = "MQTT_CONFIG: Server=" + String(MQTT_SERVER) + ":" + String(MQTT_PORT) +
                       ", User=" + String(MQTT_USER) +
                       ", Device=" + DEVICE_ID +
                       ", Listening=" + String(MQTT_TOPIC_IN) + " and " + DEVICE_MQTT_TOPIC_IN +
                       ", Responding=" + String(MQTT_TOPIC_OUT) + " and " + DEVICE_MQTT_TOPIC_OUT +
                       ", Commands=ON,<interval>,0,0,0,0,0,0 | OFF,0,0,0,0,0,0,0 | PING->PONG | HELP->ayuda";
      clientMqtt.publish(MQTT_TOPIC_OUT, mqttInfo.c_str());

      // Mostrar también por serial
      Serial.println("MQTT Configuration published:");
      Serial.println("- Server: " + String(MQTT_SERVER) + ":" + String(MQTT_PORT));
      Serial.println("- User: " + String(MQTT_USER));
      Serial.println("- Device: " + DEVICE_ID);
      Serial.println("- Listening on: " + String(MQTT_TOPIC_IN) + " and " + DEVICE_MQTT_TOPIC_IN);
      Serial.println("- Responding on: " + String(MQTT_TOPIC_OUT) + " and " + DEVICE_MQTT_TOPIC_OUT);

      // ... and resubscribe to input channels
      clientMqtt.subscribe(MQTT_TOPIC_IN); // Canal general de entrada
      clientMqtt.subscribe(DEVICE_MQTT_TOPIC_IN.c_str()); // Canal específico de entrada
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
          clientMqtt.publish(MQTT_TOPIC_OUT, ("hello world "+clientId).c_str());
          // ... and resubscribe to input channels
          clientMqtt.subscribe(MQTT_TOPIC_IN);
          extern const String DEVICE_MQTT_TOPIC_IN;
          clientMqtt.subscribe(DEVICE_MQTT_TOPIC_IN.c_str());
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

