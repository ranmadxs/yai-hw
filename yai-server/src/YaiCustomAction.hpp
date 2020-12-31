#include <Arduino.h>
#include "YaiContext.h"
#include "YaiCommons.hpp"

int pastState = 0;

void buttonCallback(int buttonRead) {
  // Reverse
  if(buttonRead == LOW) {    
    if(pastState == HIGH) {
      YaiCommand yaiCommand;
      buttonState = buttonState?false:true;
      pastState = LOW;
      yaiCommand.type = "BTN4";
      yaiCommand.command = buttonState?"ON":"OFF";
      yaiCommand.execute = EXECUTE_CMD;
      logger.debug("D4 buttonState="+String(buttonState));
      //logger.info(yaiCommand.toString());
      commandFactoryExecute(yaiCommand);
      //Serial.println("D4=" + String(buttonRead)); 
      //Serial.println("D4 pressed");
      //Serial.println("buttonState="+String(buttonState));
    }
  } else {
    pastState = HIGH;
  }


}