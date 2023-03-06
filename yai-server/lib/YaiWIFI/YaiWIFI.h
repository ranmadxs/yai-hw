#ifndef YaiWIFI_h
#define YaiWIFI_h
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <DNSServer.h>
#include <Arduino.h>
#include <WiFiClientSecure.h>

const int totalWifi = 4;
const int retryWifi = 25;

class YaiWIFI {
  public:
    
    YaiWIFI() {
      connectedWifi = false;
      apSsid = "YaiDNSServer";
    }
    String getMac(){
      return WiFi.macAddress();
    }
    void connect();
    void startDNSServer(String dnsSsid);
    bool isConnected();
    void loop();
    //WiFiClientSecure espClient;
    //BearSSL::WiFiServerSecure espClient;
    WiFiClient espClient;
    DNSServer dnsServer;
    String getIp() {
      return this->ipLocal;
    }

  private:
    const byte DNS_PORT = 53;
    String apSsid;
    String ipLocal;
    boolean connectedWifi;
    char* arrayWifi[totalWifi][2] = {
      { "YAI_LUA", "1101000000" },
      { "YAI_HUAWEI_Y9_2019", "1101000000" },
      { "VTR-YAI-5Ghz", "Pana8bc1108" } };
};

#endif