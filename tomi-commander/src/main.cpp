#include <Arduino.h>
#include "YaiCommons.h"
#include "YaiWIFI.h"
#include <PubSubClient.h>
#include "YaiMqtt.h"


const char* YAI_VERSION="0.0.1-SNAPSHOT";
// WiFiClient espClient;

void serialController();
void btnController();
void all_off();
void all_init();
void all_on();



void setup() {
  Serial.begin(115200); // opens serial port, sets data rate to 115200 bps
  existCMD = false;
  isBtnActive = false;
  Serial.println(" ####################################");
  String yaiServerVersion = " ## tomi-yai-commander v"+String(YAI_VERSION)+" ##";
	Serial.println(yaiServerVersion);
	Serial.println(" ####################################");  
  all_init();
  all_off();
  // Por defecto dejamos prendido el relay 6
  digitalWrite(NODEMCU_ARRAY_PINS[5], RelayOn);
  if (ENABLE_WIFI) { 
    Serial.println(" ######### Wifi Client ##########");
    yaiWifi.connect();
  }
  if (ENABLE_WIFI) { 
    Serial.println(" ######### DNS Server ###########");
    String dnsName = "YAI_SRV_RELAYS";
    Serial.println(dnsName);
    yaiWifi.startDNSServer(dnsName);
  }
  if (ENABLE_MQTT) { 
    clientMqtt.setServer(MQTT_SERVER, MQTT_PORT);
    clientMqtt.setCallback(callbackMqtt);
  }
  /* Init Button */
  //pinMode(ButtonPin,INPUT);
  // Configuración del pin del botón como entrada
  //pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void loop() {
  //btnController();
  serialController();

  if (ENABLE_MQTT) { 
    if (!clientMqtt.connected()) {
      reconnect();
    }
    clientMqtt.loop();
  }
}

void btnController() {
  if (digitalRead(BUTTON_PIN) == 1) {
    isBtnActive = isBtnActive?false:true;
    YaiCommand yaiCommand;
    yaiCommand.type = "BTN";
    yaiCommand.command = isBtnActive?"ON":"OFF";
    yaiCommand.execute = EXECUTE_CMD;
    yaiCommand.print = PRINT_CMD;
    commandFactoryExecute(yaiCommand);
    delay(500);
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
