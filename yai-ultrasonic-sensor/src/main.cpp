#include <Arduino.h>
#include "YaiCommons.h"
#include "YaiWIFI.h"
#include <PubSubClient.h>
#include "YaiMqtt.h"


const char* YAI_VERSION="0.0.1-SNAPSHOT";

void serialController();

void setup() {
  Serial.begin(115200); // opens serial port, sets data rate to 115200 bps
  existCMD = false;
  Serial.println(" ####################################");
  String yaiServerVersion = " ## yai-ultrasonic-sensor v"+String(YAI_VERSION)+" ##";
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
}

void loop() {
  serialController();

  if (ENABLE_MQTT) { 
    if (!clientMqtt.connected()) {
      reconnect();
    }
    clientMqtt.loop();
  }
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

