#include <Arduino.h>

//https://stackoverflow.com/questions/32135912/message-log-css-scroll-to-bottom

int buttonState=0;
int PD1 = 5;
int PD2 = 4;
int PD3 = 0;
int PD4 = 2;
int PD5 = 14;
int PD6 = 12;


void setup(void) {
	Serial.begin(115200);
  Serial.println("Serail Rdy");
  pinMode(PD4, INPUT);	
}

void loop(void) {
  buttonState=digitalRead(PD4);
  // Reverse
  if(buttonState == LOW) {
    Serial.println("D4=" + String(buttonState)); 
    Serial.println("D4 pressed");
    delay(300);
  }
}