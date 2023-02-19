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
#include "pump/YaiPumpHeight.h"

void testLittleFS();
void reconnect();
void callback(char* topic, byte* payload, unsigned int length);

YaiController yaiHttpSrv;
//YaiBtnPushDown yaiButtonPushDown(ESP_D4);
YaiPumpHeight yaiPumpHeight;


void setup(void) {
	Serial.begin(115200);	
	Serial.println("");
	Serial.println(" ###############################");
  String yaiServerVersion = " ## YaiServer v"+String(YAI_VERSION)+" ##";
	Serial.println(yaiServerVersion);
	Serial.println(" ###############################");

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
    String dnsName = "YAI_SRV_" + String(YAI_UID_NAME);
    logger.debug(dnsName);
    yaiWifi.startDNSServer(dnsName);
  }
	
  if (ENABLE_HTTP_SRV) { 
    Serial.println(" ######### HTTP Server ##########");
    yaiHttpSrv.start();
  }

  if (ENABLE_MQTT) { 
    clientMqtt.setServer(MQTT_SERVER, MQTT_PORT);
    clientMqtt.setCallback(callback);
  }

  if (ENABLE_WEBSOCKETS) { 
    Serial.println(" ######### WEBSOCKET ##########");
    InitWebSockets();
    logger.addAppender(webSocketAppender);
  }
  
  /* Init Btn4 */ 
  //yaiButtonPushDown.addCallback(buttonCallback);
  //yaiButtonPushDown.setup();

  /* Init Relay */
  //pinMode(RelayPin, OUTPUT);
  yaiPumpHeight.setLogger(logger);
  yaiPumpHeight.addCallback(webSocketAppender);
  yaiPumpHeight.setup();
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
  yaiPumpHeight.loop();
}