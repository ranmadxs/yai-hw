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
#include "YaiWebSocket.hpp"

void testLittleFS();
void reconnect();
void callback(char* topic, byte* payload, unsigned int length);

YaiController yaiHttpSrv;
YaiBtnPushDown yaiButtonPushDown(ESP_D4);

void setup(void) {
	Serial.begin(115200);	
	Serial.println("");
	Serial.println(" ###############################");
	Serial.println(" ## YaiServer v0.0.1-SNAPSHOT ##");
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
    clientMqtt.setServer(mqtt_server, 1883);
    clientMqtt.setCallback(callback);
  }

  if (ENABLE_WEBSOCKETS) { 
    Serial.println(" ######### WEBSOCKET ##########");
    InitWebSockets();
  }
  
  /* Init Btn4 */ 
  yaiButtonPushDown.addCallback(buttonCallback);
  yaiButtonPushDown.setup();

  /* Init Relay */
  pinMode(RelayPin, OUTPUT);

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
  yaiButtonPushDown.loop();
}