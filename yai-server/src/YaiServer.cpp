#include <Arduino.h>
#include <LittleFS.h>
#include <PubSubClient.h>
#include "YaiWIFI.h"
#include "YaiContext.h"
#include "YaiController.h"
#include "YaiMqtt.h"
#include "YaiFS.h"
#include "YaiActions.hpp"
#include "YaiCustomAction.hpp"
#include "YaiWebSocket.h"
#include "YaiMemory.h"
#include "pump/YaiPumpHeight.h"
#include <ArduinoJson.h>

void testLittleFS();
void reconnect();
void callback(char* topic, byte* payload, unsigned int length);

YaiController yaiHttpSrv;
YaiBtnPushDown yaiButtonPushDown(ESP_D4);
YaiPumpHeight yaiPumpHeight;
DynamicJsonDocument  docInfo(400);
YaiMemory yaiMemory;
int numCiclos = 0;
void setup(void) {
	Serial.begin(9600);
  docInfo["version"] = YAI_VERSION;
	Serial.println("");
	Serial.println(" ###############################");
  String yaiServerVersion = " ## YaiServer v"+String(YAI_VERSION)+" ##";
	Serial.println(yaiServerVersion);
	Serial.println(" ###############################");
  yaiMemory.setup();

  String wifiSSID = yaiMemory.readString(100);
  if (sizeof(wifiSSID) > 1){
    Serial.println("Wifi Selected: " + wifiSSID);
  } else {
    Serial.println("Wifi not selected");
  }
  FSInfo fs_info;
  if (LittleFS.begin()) {
		Serial.println("LittleFS ready");
	}
  LittleFS.info(fs_info);

  testLittleFS();
  
  if (ENABLE_WIFI) { 
    Serial.println(" ######### Wifi Client ##########");
    yaiWifi.connect();
  }
  if (ENABLE_WIFI) { 
    Serial.println(" ######### DNS Server ###########");
    String dnsName = "YAI_SRV_";
    logger.debug(dnsName);
    yaiWifi.startDNSServer(dnsName);
    docInfo["mac"] = yaiWifi.getMac();
    docInfo["local_ip"] = yaiWifi.getIp();
  }
	
  if (ENABLE_HTTP_SRV) { 
    Serial.println(" ######### HTTP Server ##########");
    yaiHttpSrv.start();
  }

  if (ENABLE_MQTT) { 
    clientMqtt.setServer(MQTT_SERVER, MQTT_PORT);
    yaiWifi.addAppender(webSocketAppender);
  }

  if (ENABLE_WEBSOCKETS) { 
    Serial.println(" ######### WEBSOCKET ##########");
    InitWebSockets();
    logger.addAppender(webSocketAppender);
    yaiPumpHeight.addCallback(webSocketAppender);
  }
  
  /* Init Btn4 */ 
  //yaiButtonPushDown.addCallback(buttonCallback);
  //yaiButtonPushDown.setup();

  /* Init Relay */
  //pinMode(RelayPin, OUTPUT);
  if(ENABLE_YAI_PUMP_HEIGHT) {
    yaiPumpHeight.setLogger(logger);
    yaiPumpHeight.addCallback(webSocketAppender);
    yaiPumpHeight.addCallback(mqttCallback);
    yaiPumpHeight.setDocInfo(&docInfo);
    yaiPumpHeight.setup();
  } 
  logger.info("Ready");
}

void loop(void) {
  if (ENABLE_MQTT) { 
    if (!clientMqtt.connected()) {
      reconnect();
    }
    clientMqtt.loop();
  }
  if (ENABLE_WEBSOCKETS) { 
    webSocket.loop();
  }
  //yaiButtonPushDown.loop();
  if(ENABLE_YAI_PUMP_HEIGHT) {
    yaiPumpHeight.loop();
  }

  if(ENABLE_WIFI) {
    numCiclos = numCiclos + 1;
    if(numCiclos == 18000) {
      yaiWifi.scanNetworks();
      numCiclos = 0;
    }
  }
}