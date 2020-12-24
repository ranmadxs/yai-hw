#ifndef YaiFS_h
#define YaiFS_h
#include <Arduino.h>
#include <LittleFS.h>

void testLittleFS(){

  Serial.println("----------InDir /------------");
  Dir dir2 = LittleFS.openDir("/");
  while (dir2.next()) {
    Serial.print(dir2.fileName());
    if(dir2.fileSize()) {
        File f2 = dir2.openFile("r");
        Serial.println("  " + String(f2.size()) + "   " + f2.getCreationTime());
    } else {
      Serial.println("  <Dir>");
    }
  }  

  Dir dir = LittleFS.openDir("/html");
  Serial.println("-----------/html-------------");
  while (dir.next()) {
    Serial.print(dir.fileName());
    if(dir.fileSize()) {
        File f = dir.openFile("r");
        Serial.println(f.size());
    }
  }


  Serial.println("----------[/html/index.html]-------------");

  File dataFile = LittleFS.open("/html/index.html", "r");
  
  while(dataFile.available()){
    Serial.write(dataFile.read());
  }
  Serial.println("------------------------------");
}

#endif