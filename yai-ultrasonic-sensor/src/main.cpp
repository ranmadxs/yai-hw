#include <Arduino.h>
#include "YaiCommons.h"
#include "YaiWIFI.h"
#include <PubSubClient.h>
#include "YaiMqtt.h"
#include "YaiUltrasonicSensor.h"

const char* YAI_VERSION="0.2.2-SNAPSHOT";

// Device ID estático para el sensor
const String DEVICE_ID = "YUS-" + String(YAI_VERSION);

// Canal MQTT específico del dispositivo
const String DEVICE_MQTT_TOPIC = "yai-mqtt/" + DEVICE_ID + "/out";

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

