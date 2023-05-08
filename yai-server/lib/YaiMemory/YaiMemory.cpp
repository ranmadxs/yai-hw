#include "YaiMemory.h"

void YaiMemory::setup() {
    EEPROM.begin(this->EEPROM_SIZE);
}

void YaiMemory::writeString(int addrOffset, const String &str) {
    byte len = str.length();
    EEPROM.write(addrOffset, len);
    for (int i = 0; i < len; i++) {
        EEPROM.write(addrOffset + 1 + i, str[i]);
    }    
}

String YaiMemory::readString(int addrOffset) {
    float testAddress;
    EEPROM.get( addrOffset, testAddress );    
    if(testAddress == NAN) {
        return "";
    } else {
        Serial.println(testAddress);
        int newStrLen = EEPROM.read(addrOffset);
        char data[newStrLen + 1];
        for (int i = 0; i < newStrLen; i++) {
            data[i] = EEPROM.read(addrOffset + 1 + i);
        }
        //data[newStrLen] = '\ 0'; 
        // !!! NOTE !!! Remove the space between the slash "/" and "0" (I've added a space because otherwise there is a display bug)
        return String(data);
    }
}