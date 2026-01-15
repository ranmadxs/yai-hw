#include <Arduino.h>
#include "YaiCommons.h"
#include "YaiWIFI.h"
#include <PubSubClient.h>
#include "YaiMqtt.h"
#include "YaiUltrasonicSensor.h"

const char* YAI_VERSION="0.2.7-SNAPSHOT";

// Device ID estático para el sensor
const String DEVICE_ID = "YUS-" + String(YAI_VERSION);

// Canales MQTT específicos del dispositivo
const String DEVICE_MQTT_TOPIC_OUT = "yai-mqtt/" + DEVICE_ID + "/out";
const String DEVICE_MQTT_TOPIC_IN = "yai-mqtt/" + DEVICE_ID + "/in";

// Variables globales para controlar logs del sensor
bool ultrasonicLogsEnabled = true;  // Logs habilitados por defecto
unsigned long ultrasonicMeasurementInterval = 1500; // Intervalo por defecto (1.5 segundos)

// Definición de pines del sensor ultrasónico
const int PIN_TRIG = 5;  // Enviar pulso
const int PIN_ECHO = 18; // Recibir respuesta

// Instancia del sensor ultrasónico
YaiUltrasonicSensor sensorUltrasonico(PIN_TRIG, PIN_ECHO, ultrasonicMeasurementInterval);

void serialController();

void setup() {
  Serial.begin(115200); // opens serial port, sets data rate to 115200 bps
  existCMD = false;
  Serial.println(" ####################################");
  String yaiServerVersion = " ## yai-ultrasonic-sensor " + DEVICE_ID + " ##";
	Serial.println(yaiServerVersion);
	Serial.println(" ####################################");  
  if (ENABLE_WIFI) { 
    Serial.println(" ######### Wifi Client ##########");
    yaiWifi.connect();
  }
  if (ENABLE_WIFI) { 
    Serial.println(" ######### DNS Server ###########");
    String dnsName = "YAI_SRV_ULTRASONIC";
    Serial.println(dnsName);
    yaiWifi.startDNSServer(dnsName);
  }
  if (ENABLE_MQTT) { 
    clientMqtt.setServer(MQTT_SERVER, MQTT_PORT);
    clientMqtt.setCallback(callbackMqtt);
  }
  
  // Inicializamos el sensor ultrasónico
  sensorUltrasonico.begin();
  
  // Configuramos MQTT para el sensor si está habilitado
  if (ENABLE_MQTT) {
    sensorUltrasonico.setMqttClient(&clientMqtt);
    sensorUltrasonico.setMqttTopic(MQTT_TOPIC_OUT);

    // Publicar información de configuración MQTT al iniciar
    String mqttInfo = "MQTT_CONFIG: Server=" + String(MQTT_SERVER) + ":" + String(MQTT_PORT) +
                     ", User=" + String(MQTT_USER) +
                     ", Device=" + DEVICE_ID +
                     ", Listening=" + String(MQTT_TOPIC_IN) + " and " + DEVICE_MQTT_TOPIC_IN +
                     ", Responding=" + String(MQTT_TOPIC_OUT) + " and " + DEVICE_MQTT_TOPIC_OUT +
                     ", Commands=ON,<interval>,0,0,0,0,0,0 | OFF,0,0,0,0,0,0,0 | PING->PONG";

    if (clientMqtt.connected()) {
      clientMqtt.publish(MQTT_TOPIC_OUT, mqttInfo.c_str());
      Serial.println("MQTT Configuration published on startup:");
      Serial.println("- Server: " + String(MQTT_SERVER) + ":" + String(MQTT_PORT));
      Serial.println("- User: " + String(MQTT_USER));
      Serial.println("- Device: " + DEVICE_ID);
      Serial.println("- Listening on: " + String(MQTT_TOPIC_IN) + " and " + DEVICE_MQTT_TOPIC_IN);
      Serial.println("- Responding on: " + String(MQTT_TOPIC_OUT) + " and " + DEVICE_MQTT_TOPIC_OUT);
    }
  }

}

void loop() {
  serialController();

  if (ENABLE_MQTT) { 
    if (!clientMqtt.connected()) {
      reconnect();
    }
    clientMqtt.loop();
  }
  
  // Loop del sensor ultrasónico (maneja lectura y envío MQTT)
  sensorUltrasonico.loop();
}

void serialController() {
	YaiCommand yaiResCmd;
	YaiCommand yaiCommand;
	yaiCommand = yaiUtil.commandSerialFilter();
  if (String(YAI_COMMAND_TYPE_SERIAL) == yaiCommand.type) {
    // Serial.println(yaiCommand.toString());
    yaiCommand.execute = EXECUTE_CMD;
    yaiCommand.print = PRINT_CMD;
    commandFactoryExecute(yaiCommand);
  }	
	// if(yaiResCmd.type == String(YAI_COMMAND_TYPE_RESULT)){
		// Serial.print(">> ");
		// Serial.println(yaiResCmd.json);
	// }
}

