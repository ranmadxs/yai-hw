#include <Arduino.h>
#include "YaiCommons.h"
#include "YaiWIFI.h"
#include <PubSubClient.h>
#include "YaiMqtt.h"
#include "YaiUltrasonicSensor.h"
#include <NTPClient.h>
#include <WiFiUdp.h>

#if defined(ESP32)
#include <esp_system.h>
#elif defined(ESP8266)
#include <ESP.h>
#endif

const char* YAI_VERSION="0.3.1-COSTA";

// Genera un ID corto basado en el chip (8 hex) para usar en los canales MQTT
String getChipShortId() {
  char id[9];
#if defined(ESP32)
  uint64_t chipid = ESP.getEfuseMac();
  uint32_t low = (uint32_t)(chipid & 0xFFFFFFFF);
  sprintf(id, "%08X", low);
#elif defined(ESP8266)
  uint32_t chipid = ESP.getChipId();
  sprintf(id, "%08X", chipid);
#else
  sprintf(id, "%08X", 0xFFFFFFFF);
#endif
  return String(id);
}

// Identificador de canal MQTT basado en el chip (ej: 1A2B3C4D), longitud máx. 8
const String CHANNEL_ID = getChipShortId();

// Device ID estático para el sensor (incluye versión, se usa en mensajes)
const String DEVICE_ID = "YUS-" + String(YAI_VERSION);

// Canales MQTT específicos del dispositivo (usan CHANNEL_ID, no DEVICE_ID)
const String DEVICE_MQTT_TOPIC_OUT = "yai-mqtt/" + CHANNEL_ID + "/out";
const String DEVICE_MQTT_TOPIC_IN = "yai-mqtt/" + CHANNEL_ID + "/in";

// Variables globales para controlar logs del sensor
bool ultrasonicLogsEnabled = true;  // Logs habilitados por defecto
unsigned long ultrasonicMeasurementInterval = 1500; // Intervalo por defecto (1.5 segundos)

// Definición de pines del sensor ultrasónico
const int PIN_TRIG = 5;  // Enviar pulso
const int PIN_ECHO = 18; // Recibir respuesta

// Instancia del sensor ultrasónico
YaiUltrasonicSensor sensorUltrasonico(PIN_TRIG, PIN_ECHO, ultrasonicMeasurementInterval);

// Cliente NTP para obtener la hora actual
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -3 * 3600, 60000); // UTC-3 (Argentina), actualizar cada 60 segundos

// Función para obtener timestamp formateado (NTP si conectado, millis si no)
String getCurrentTimestamp() {
  if (ENABLE_WIFI && WiFi.status() == WL_CONNECTED) {
    // Usar NTP si WiFi está conectado
    time_t epochTime = timeClient.getEpochTime();
    struct tm *ptm = gmtime(&epochTime);
    char timestamp[20];
    sprintf(timestamp, "%04d-%02d-%02d %02d:%02d:%02d",
            ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday,
            ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
    return String(timestamp);
  } else {
    // Usar millis si no hay WiFi
    return String(millis());
  }
}

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

    // Inicializar NTP Client
    Serial.println(" ######### NTP Client ###########");
    timeClient.begin();
    timeClient.update();
    Serial.println("NTP initialized");
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
                     ", Commands=ON,<interval>,0,0,0,0,0,0 | OFF,0,0,0,0,0,0,0 | PING->PONG | HELP->ayuda";

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

  // Actualizar NTP si WiFi está habilitado
  if (ENABLE_WIFI) {
    timeClient.update();
  }

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

