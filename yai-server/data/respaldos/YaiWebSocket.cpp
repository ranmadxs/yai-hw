#include "YaiWebSocket.h"
      void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {
         switch(type) {
         case WStype_DISCONNECTED:
            break;
         case WStype_CONNECTED:
            //IPAddress ip = webSocket.remoteIP(num);
            //webSocket.sendTXT(num, "Connected");
            break;
         case WStype_TEXT:
            break;
         }
      }  
      void webSocketAppender(String msg) {
         //webSocket.broadcastTXT(msg);
      }

void YaiWebSocket::initWebSockets() {
         this->webSocket.begin();
         webSocket.onEvent(webSocketEvent);
         logger.addAppender(webSocketAppender);
         Serial.println("WebSocket server started in port:" + String(81));
      }      