#include "YaiHttpSrv.h"

void onRequestNotFound(AsyncWebServerRequest *request){
  String content_type = "text/plain";
  String message = "404 File Not Found\n\n";
  int codeStatus = 404;        
  message += "URI: ";
  message += request->url();
  message += "\nMethod: ";
  message += request->methodToString();
  message += "\nArguments: ";
  message += request->args();
  message += "\n";
  for (uint8_t i = 0; i < request->args(); i++) {
    message += " " + request->argName(i) + ": " + request->arg(i) + "\n";
  }
  request->send(404, content_type, message);   
}

void YaiHttpSrv::httpController() {
  httpYaiServer->on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "YaiHttpSrv Ready, must implement YaiHttpSrv::httpController");
  });
}

void YaiHttpSrv::start(){
  httpController();
  httpYaiServer->onNotFound(onRequestNotFound);
	httpYaiServer->begin();
	Serial.println("HTTP server started");   
}