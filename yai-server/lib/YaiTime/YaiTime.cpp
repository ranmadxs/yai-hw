#include "YaiTime.hpp"

void YaiTime::printLocalTime() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println("%A, %B %d %Y %H:%M:%S");
}

void YaiTime::setup() {
  //init and get the time
  configTime(this->gmtOffset_sec, this->daylightOffset_sec, this->ntpServer);
  this->printLocalTime();
}