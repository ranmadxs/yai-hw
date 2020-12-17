#include <Arduino.h>
#include "YaiCommons.h"

String serialCMD;
bool existCMD;

#define RelayOn         HIGH
#define RelayOff        LOW
#define RelayPin        6
#define ButtonPin       2
#define PRINT_CMD       true
#define EXECUTE_CMD     true
bool isBtnActive;
YaiUtil yaiUtil;

void WaterPumpOn();
void WaterPumpOff();
void serialController();
void btnController();

void setup() {
  Serial.begin(9600); // opens serial port, sets data rate to 9600 bps
  existCMD = false;
  isBtnActive = false;
  /* Init Relay */
  pinMode(RelayPin, OUTPUT);  
  /* Init Button */
  pinMode(ButtonPin,INPUT);
}

void loop() {
  btnController();
  serialController();
}

void commandFactoryExecute(YaiCommand yaiCommand) {
	YaiCommand yaiResCmd;
	if(yaiCommand.print){
  	Serial.print("<< ");  //-> Esto se transforma en log debug
		Serial.println(yaiCommand.toString());
  }
	if (yaiCommand.execute) {
    existCMD = false;
    if (yaiCommand.command == "ON") {
      Serial.println("POWER ON");
      existCMD = true;
      isBtnActive = true;
      digitalWrite(RelayPin, RelayOn);
    }
    if (yaiCommand.command == "OFF") {
      existCMD = true;
      isBtnActive = false;
      Serial.println("POWER OFF");
      digitalWrite(RelayPin, RelayOff);
    }      
    if (!existCMD) {
      yaiCommand.error = yaiCommand.command + " command not found";
    }		
	} else {
    Serial.println("[WARN] Not execute command " + yaiCommand.command);
  } 
  if( yaiCommand.error.length() > 1 ) {
    Serial.println("[ERROR] " + yaiCommand.error);
  }
}

void btnController() {
  if (digitalRead(ButtonPin) == 1) {
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

void WaterPumpOn() {
  digitalWrite(RelayPin, RelayOn);
}

void WaterPumpOff() {
  digitalWrite(RelayPin, RelayOff);
}