#ifndef YaiHttpSrv_h
#define YaiHttpSrv_h

#include <ESPAsyncWebServer.h>
#include <ESPAsyncTCP.h>

//https://github.com/me-no-dev/ESPAsyncWebServer#respond-with-content-coming-from-a-stream

class YaiHttpSrv {
  public:
         
   
    YaiHttpSrv() {
      httpYaiServer = new AsyncWebServer(80);
    }

    void start();    

    virtual void httpController();

    AsyncWebServer* getServer(){
      return httpYaiServer;
    }

  private:
    AsyncWebServer* httpYaiServer;
};

#endif