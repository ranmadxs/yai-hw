#ifndef YaiController_h
#define YaiController_h
#include "YaiContext.h"
#include "YaiHttpSrv.h"
#include <LittleFS.h>

String processor(const String& var){
  if (var == "YAI_IP")
    return "YAI_IP Value";
  if (var == "YAI_UID")
    return YAI_UID_NAME;
  if (var == "YAI_VERSION")
    return YAI_VERSION;    
  return "0";
}



class YaiController : public YaiHttpSrv {
  public:

    YaiController(){};


    void httpController() override {
      
      getServer()->on("/acercaDe", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "YaiController Ready");
      });

      getServer()->on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(LittleFS, "/index.html", String(), false, processor);
      });
      /*
      getServer()->on("/js/websocketClient.js", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(LittleFS, "/js/websocketClient.js", String(), false, processor);
      });

      getServer()->on("/js/jquery.js", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(LittleFS, "/js/jquery.js", String(), false, processor);
      });
      */

      getServer()->serveStatic("/css", LittleFS, "/css");
      getServer()->serveStatic("/js", LittleFS, "/js");
      /*
      getServer()->on("/css/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(LittleFS, "/css/style.css", String(), false, processor);
      });
      */
      getServer()->on("/cmd", HTTP_GET, [](AsyncWebServerRequest *request){
				String message = "";
				String jsonCommand = "";
				for (uint8_t i=0; i<request->args(); i++) {
					jsonCommand += request->arg(i);
					if(i + 1 < request->args()) {
						jsonCommand += ",";
					}
				}
        logger.debug(jsonCommand);
				message += " << " + jsonCommand;
				message += "\n";
        AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", message);
				response->addHeader("Access-Control-Allow-Origin", "*");
				response->addHeader("Access-Control-Allow-Methods", "POST,GET,OPTIONS");
				response->addHeader("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");
        response->addHeader("Server","ESP Async Web Server");
        request->send(response);
			});           
    }
};

#endif