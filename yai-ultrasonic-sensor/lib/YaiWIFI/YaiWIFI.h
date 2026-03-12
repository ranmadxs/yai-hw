#ifndef YaiWIFI_h
#define YaiWIFI_h
#include <Arduino.h>
#include <ArduinoJson.h>

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266mDNS.h>
  #include <DNSServer.h>
  #include <EEPROM.h>
#elif defined(ESP32)
  #include <WiFi.h>
  #include <ESPmDNS.h>
  #include <DNSServer.h>
  #include <Preferences.h>
#else
  #error "Plataforma no soportada. Solo ESP8266 y ESP32 son compatibles."
#endif

#include <WiFiClient.h>
#include <WiFiClientSecure.h>

#define YAI_WIFI_STORED_SSID_MAX 32
#define YAI_WIFI_STORED_PASS_MAX 64

const int totalWifi = 5;
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
    /** Guarda ssid/password en flash. Se usa en próximo boot si existe. */
    void saveStoredWifi(const char* ssid, const char* password);
    /** true si hay credenciales guardadas en memoria */
    bool hasStoredWifi();
    /** Borra las credenciales guardadas. Próximo boot usará lista hardcodeada. */
    void clearStoredWifi();
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
      { "YaiCosta", "1101000000" },
      { "Patitas", "21193120" },
      { "Patitas-EXT", "21193120" },
      { "YAI_LUA", "1101000000" },
      { "VTR-YAI-5Ghz", "Pana8bc1108" } };

  protected:
    YaiWIFICallBack callbacks[10];
    int totalAppender;
};

#endif

