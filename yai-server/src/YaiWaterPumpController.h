#ifndef YaiWaterPumpController_h
#define YaiWaterPumpController_h

#include "YaiHttpSrv.h"

class YaiWaterPumpController : public YaiHttpSrv {
  public:

    YaiWaterPumpController(){};
    
    void httpController() override {
      getServer()->on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "YaiWaterPumpController Ready");
      });
    }
};

#endif