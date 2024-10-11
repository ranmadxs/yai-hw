#ifndef YaiWIFI_h
#define YaiWIFI_h
#include <WiFi.h>
#include <WiFiClient.h>
#include <DNSServer.h>
#include <Arduino.h>
#include <ESPmDNS.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

const int totalWifi = 4;
const int retryWifi = 25;

class YaiWIFICallBack {
  public:
    ~YaiWIFICallBack(){}
    YaiWIFICallBack(void (*f)(String) = 0)
        : function(f) {}
    void (*function)(String);
};

class YaiWIFI {
  public:
    
    YaiWIFI() {
      connectedWifi = false;
      apSsid = "YaiRelayDNSServer";
    }
    String getMac(){
      return WiFi.macAddress();
    }
    void connect();
    void startDNSServer(String dnsSsid);
    bool isConnected();
    void loop();
    void scanNetworks();
    //WiFiClientSecure espClient;
    //BearSSL::WiFiServerSecure espClient;
    WiFiClient espClient;
    DNSServer dnsServer;
    void addAppender(YaiWIFICallBack lolaso);
    String getIp() {
      return this->ipLocal;
    }

  private:
    const byte DNS_PORT = 53;
    String apSsid;
    String ipLocal;
    boolean connectedWifi;
    char* arrayWifi[totalWifi][2] = {
      { "Patitas-EXT", "21193120" },
      { "Patitas", "21193120" },
      { "YAI_LUA", "1101000000" },
      { "VTR-YAI-5Ghz", "Pana8bc1108" } };

  protected:
    YaiWIFICallBack callbacks[10];
    int totalAppender;
};

#endif