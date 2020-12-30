#include <Arduino.h>
bool buttonState = false;
int pastState = 0;

void buttonCallback(int buttonRead) {
  // Reverse
  if(buttonRead == LOW) {
    if(pastState == HIGH) {
      buttonState = buttonState?false:true;
      pastState = LOW;
      Serial.println("D4=" + String(buttonRead)); 
      Serial.println("D4 pressed");
      Serial.println("buttonState="+String(buttonState));
    }
  } else {
    pastState = HIGH;
  }


}