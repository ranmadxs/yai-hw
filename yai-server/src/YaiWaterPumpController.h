#ifndef YaiWaterPumpController_h
#define YaiWaterPumpController_h

#include "YaiHttpSrv.h"

  String processor(const String& var){
    Serial.print("Processor: ");
    Serial.println(var);
    if (var == "YAI_IP")
      return "YAI_IP Value";
    if (var == "YAI_UID")
      return "YAI_UID Value";
  }


class YaiWaterPumpController : public YaiHttpSrv {
  public:

    YaiWaterPumpController(){};


    void httpController() override {
      getServer()->on("/acercaDe", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "YaiWaterPumpController Ready");
      });
      getServer()->on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(LittleFS, "/html/index.html", String(), false, processor);
      });       
    }
};

#endif