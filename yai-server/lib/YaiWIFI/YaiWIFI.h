#ifndef YaiWIFI_h
#define YaiWIFI_h
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <DNSServer.h>
#include <Arduino.h>

const int totalWifi = 3;
const int retryWifi = 17;

class YaiWIFI {
  public:
    
    YaiWIFI() {
      connectedWifi = false;
      apSsid = "YaiDNSServer";
      WiFi.mode(WIFI_AP_STA);
    }

    void connect();
    void startDNSServer(String dnsSsid);

    WiFiClient espClient;
    DNSServer dnsServer;

  private:
    const byte DNS_PORT = 53;
    String apSsid;    
    boolean connectedWifi;
    char* arrayWifi[totalWifi][2] = {
      { "YAI HUAWEI Y9 2019", "1101000000" },
      { "Cachantun*", "2119amto" },
      { "VTR-YAI-5Ghz", "Pana8bc1108" } };
};

#endif