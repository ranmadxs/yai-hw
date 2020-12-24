
#include <Arduino.h>
#include <LittleFS.h>
#include <PubSubClient.h>
#include "YaiWIFI.h"
#include "YaiContext.h"
#include "YaiController.h"
#include "YaiMqtt.h"
#include "YaiFS.h"

void testLittleFS();
void reconnect();
void callback(char* topic, byte* payload, unsigned int length);

YaiController yaiHttpSrv;

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
    yaiWifi.startDNSServer("YAI_SRV_" + YAI_UID);
  }
	
  if (ENABLE_HTTP_SRV) { 
    Serial.println(" ######### HTTP Server ##########");
    yaiHttpSrv.start();
  }

  clientMqtt.setServer(mqtt_server, 1883);
  clientMqtt.setCallback(callback);
}

void loop(void) {
  if (!clientMqtt.connected()) {
    reconnect();
  }
  clientMqtt.loop();
}