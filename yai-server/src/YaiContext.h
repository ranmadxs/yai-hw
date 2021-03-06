#ifndef YaiContext_h
#define YaiContext_h
#include <Arduino.h>
#include <LittleFS.h>
#include <PubSubClient.h>
#include "YaiWIFI.h"
#include "YaiCommons.hpp"

const char* YAI_UID_NAME = "WP01";

int ESP_D4 = 2;
int ESP_D6 = 12;

#define RelayPin  ESP_D6
#define RelayOn   HIGH
#define RelayOff  LOW      
bool buttonState = false;

const int WEBSOCKET_PORT = 81;

#define EXECUTE_CMD     true
char message_buff[100];
YaiLog logger(YAI_UID_NAME);
//const String YAI_UID = "WP01";

const char* mqtt_server = "broker.hivemq.com";
//const char* mqtt_server = "192.168.1.40";
const char* mqtt_user = "test";
const char* mqtt_password = "test";

const bool ENABLE_WIFI = true;
const bool ENABLE_HTTP_SRV = true;
const bool ENABLE_MQTT = true && ENABLE_WIFI;
const bool ENABLE_WEBSOCKETS = true && ENABLE_WIFI;
//AsyncWebServer server(80);

// WiFiClient espClient;
YaiWIFI yaiWifi;
PubSubClient clientMqtt(yaiWifi.espClient);
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];


void commandFactoryExecute(YaiCommand yaiCommand) {
	YaiCommand yaiResCmd;
  bool existCMD = false;
	if(yaiCommand.print){
		logger.debug("<<" + yaiCommand.toString());
  }
	if (yaiCommand.execute) {
    existCMD = false;
    if (yaiCommand.command == "ON") {
      logger.debug("POWER ON");
      existCMD = true;
      buttonState = true;
      digitalWrite(RelayPin, RelayOn);
    }
    if (yaiCommand.command == "OFF") {
      existCMD = true;
      buttonState = false;
      logger.debug("POWER OFF");
      digitalWrite(RelayPin, RelayOff);
    }      
    if (!existCMD) {
      yaiCommand.error = yaiCommand.command + " command not found";
    }		
	} else {
   logger.warn("Not execute command " + yaiCommand.command);
  } 
  if( yaiCommand.error.length() > 1 ) {
    logger.error(yaiCommand.error);
  }
}

#endif