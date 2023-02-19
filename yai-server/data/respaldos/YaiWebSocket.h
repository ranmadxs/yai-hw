#ifndef YaiWebSocket_h
#define YaiWebSocket_h
#include <Arduino.h>
//#include "YaiContext.h"
#include <WebSocketsServer.h>
#include "YaiCommons.hpp"

class YaiWebSocket {
   public:
      YaiWebSocket(){};
      void initWebSockets();
      void webSocketAppender(String msg) {
         webSocket.broadcastTXT(msg);
      }
      void setLogger(YaiLog _logger) {
         this->logger = _logger;
      }            
   private:
      WebSocketsServer webSocket = WebSocketsServer(81);
      YaiLog logger;
   
};

#endif