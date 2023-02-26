#include "YaiWIFI.h"

IPAddress apLocalIp(192, 168, 50, 1);
IPAddress apSubnetMask(255, 255, 255, 0);

bool YaiWIFI::isConnected() {
  return WiFi.isConnected();
}

void YaiWIFI::loop() {
  if(!this->isConnected()){
    Serial.print("Reconecting");
    WiFi.reconnect();
  }
}

void YaiWIFI::connect() {
  char* ssid;
  char* password;
  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);  
  for (int j = 0; j < totalWifi; j++) {
    Serial.print("Conectando a " + String(arrayWifi[j][0]) + " ");
    ssid = arrayWifi[j][0];
    password = arrayWifi[j][1];

    WiFi.begin(ssid, password);
    for (int k = 0; k < retryWifi; k++) {
      if (WiFi.status() == WL_CONNECTED) {
        k = retryWifi;
        j = totalWifi;
        connectedWifi = true;
        Serial.print(" Connected!!!");
      } else {
        delay(500);
        Serial.print(".");
      }
    }
    Serial.println("");
  }
  String yaiIP = WiFi.localIP().toString();
  if (!connectedWifi) {
    ssid = "None";
  }
  Serial.print("Connected to: ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(yaiIP);
  Serial.print("MAC address: ");
  Serial.println(WiFi.macAddress());  
}

void YaiWIFI::startDNSServer(String dnsSsid) {
  apSsid = dnsSsid;
  WiFi.softAPConfig(apLocalIp, apLocalIp, apSubnetMask);
  WiFi.softAP(apSsid);
  dnsServer.setTTL(300);
  dnsServer.setErrorReplyCode(DNSReplyCode::ServerFailure);
  dnsServer.start(DNS_PORT, "yairover.ddns.com", apLocalIp);
  Serial.println("DNS Server OK ip:" + apLocalIp.toString() );
  Serial.println("ssid:" + String(apSsid));  
  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }
  MDNS.addService("http", "tcp", 80);    
}