#ifndef YaiController_h
#define YaiController_h
#include "YaiContext.h"
#include "YaiHttpSrv.h"
#include <LittleFS.h>

String processor(const String& var){
  Serial.print("Processor: ");
  Serial.println(var);
  if (var == "YAI_IP")
    return "YAI_IP Value";
  if (var == "YAI_UID")
    return YAI_UID;
}

class YaiController : public YaiHttpSrv {
  public:

    YaiController(){};


    void httpController() override {
      getServer()->on("/acercaDe", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "YaiController Ready");
      });
      getServer()->on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(LittleFS, "/html/index.html", String(), false, processor);
      });       
    }
};

#endif