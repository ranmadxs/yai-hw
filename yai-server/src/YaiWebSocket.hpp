#include "YaiContext.h"
#include <WebSocketsServer.h>

WebSocketsServer webSocket = WebSocketsServer(81);

String GetMillis() {
   return String(millis(), DEC);
}
 
String ProcessRequest() {
   return "Milisegundos=" + GetMillis();
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {
   switch(type) {
   case WStype_DISCONNECTED:
      break;
   case WStype_CONNECTED:
      //IPAddress ip = webSocket.remoteIP(num);
      //webSocket.sendTXT(num, "Connected");
      break;
   case WStype_TEXT:
      String response = ProcessRequest();
      webSocket.sendTXT(num, response);
      break;
   }
}

void webSocketAppender(String msg) {
  webSocket.broadcastTXT(msg);
}

void InitWebSockets() {
   webSocket.begin();
   webSocket.onEvent(webSocketEvent);
   logger.addAppender(webSocketAppender);
   Serial.println("WebSocket server started in port:" + String(WEBSOCKET_PORT));
}