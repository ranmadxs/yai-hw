#include <Arduino.h>

//https://stackoverflow.com/questions/32135912/message-log-css-scroll-to-bottom

bool buttonState = false;
int pastState = 0;
int buttonRead=0;
int PD0 = 16;
int PD1 = 5;  // SCL (I2C)
int PD2 = 4;  // SDA (I2C)
int PD3 = 0;
int PD4 = 2;  // TXD1
int PD5 = 14; // SCLK
int PD6 = 12; // MISO -> RDX2
int PD7 = 13; // MOSI -> TXD2
int PD8 = 15; // CS
int PRX = 3; // RXD0
int TRX = 1; // TXD0


#define ButtonPin  PD4
#define ButtonOn   LOW
#define ButtonOff  HIGH

#define RelayPin  PD5
#define RelayOn   LOW
#define RelayOff  HIGH

void setup(void) {
	Serial.begin(115200);
  Serial.println("Serail Rdy");
  pinMode(PD4, INPUT);
}

void loop(void) {
  buttonRead=digitalRead(ButtonPin);
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