#ifndef YaiMemory_h
#define YaiMemory_h
#include <Arduino.h>
#include <EEPROM.h>

class YaiMemory {
  public:
    void setup();
    void writeString(int addrOffset, const String &str);
    String readString(int addrOffset);

  private:
    int EEPROM_SIZE = 12;
};

#endif