#ifndef YaiWIFI_h
#define YaiWIFI_h
#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266mDNS.h>
#elif defined(ESP32)
  #include <WiFi.h>
  #include <ESPmDNS.h>
#else
  #error "Plataforma no soportada"
#endif
#include <WiFiClient.h>
#include <DNSServer.h>
#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

const int totalWifi = 2;
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
      { "Patitas", "21193120" } };

  protected:
    YaiWIFICallBack callbacks[10];
    int totalAppender;
};

#endif