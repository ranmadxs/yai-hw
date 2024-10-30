#include "YaiWIFI.h"

IPAddress apLocalIp(192, 168, 50, 1);
IPAddress apSubnetMask(255, 255, 255, 0);

/****** root certificate *********/

static const char *root_ca PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)EOF";

static const char client_cert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDWjCCAkKgAwIBAgIVAOFlUZIDMrUht5zbLrjKog1uWrUKMA0GCSqGSIb3DQEB
CwUAME0xSzBJBgNVBAsMQkFtYXpvbiBXZWIgU2VydmljZXMgTz1BbWF6b24uY29t
...
mJKLnqz8EkxQB6qd2/7XimHrmYoo/DI1KjHKfxEFxUnwkOp1wC6sh78bPXordDTL
gBWeKhwdZHZ3d6TdkY+tpmbSE13+n1+4kUit901F4NIDecdMlIN3zJqQwTPsyw==
-----END CERTIFICATE-----
)EOF";

static const char client_key[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
MIIEowIBAAKCAQEAvdXSakGZNJoGlnY7B/Q7FZNRToZzl5QUAYaOvxZRawnKWfd0
b70e7rSSphbf+GNtvKM7S2iXtMH9qz3Vmv7qVM0CxfwzFi//z3m3ncmrqOSkLSuT
....
ut8HxDMV+AmxMDPTnC+5dEhduIw8r1mMBiYr7TXcu5vnDOgzlCq+S6RyWuOVcF2m
YJ8BWgR5mp4KsuPj/eczZLnKgtDzVLoGjg5VE4dPYKypBYHzeeqX
-----END RSA PRIVATE KEY-----
)KEY";


void YaiWIFI::addAppender(YaiWIFICallBack lolaso){
  callbacks[totalAppender] = lolaso;
  totalAppender++;
}

bool YaiWIFI::isConnected() {
  return WiFi.isConnected();
}

void YaiWIFI::loop() {
  if(!this->isConnected()){
    Serial.print("Reconecting");
    this->connect();
  }
}

void YaiWIFI::connect() {
  char* ssid;
  char* password;
  //WiFi.mode(WIFI_STA);
  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);
  //BearSSL::WiFiClientSecure client;
  //BearSSL::X509List cert(root_ca);
  //this->espClient.setTrustAnchors(&cert);
  //this->espClient.setTrustAnchors(&cert);
  //BearSSL::X509List client_crt(client_cert);
  //BearSSL::PrivateKey key(client_key);
  //this->espClient.setClientRSACert(&client_crt, &key);
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
  String yaiIPLocal = WiFi.localIP().toString();
  if (!connectedWifi) {
    ssid = "None";
  }
  Serial.print("Connected to: ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  this->ipLocal = yaiIPLocal;
  Serial.println(yaiIPLocal);
  Serial.print("MAC address: ");
  Serial.println(WiFi.macAddress());  
}

void YaiWIFI::startDNSServer(String dnsSsid) {
  String wifiBaseName = WiFi.macAddress();
  wifiBaseName.replace(":", "");
  apSsid = dnsSsid + wifiBaseName;
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

void YaiWIFI::scanNetworks() {
  int numberOfNetworks = WiFi.scanNetworks();
  DynamicJsonDocument  doc(500);
  DynamicJsonDocument docValueInfo(400);
  JsonArray networksArray = doc.createNestedArray("networks");
  
  for(int i =0; i<numberOfNetworks; i++) {
      docValueInfo["ssid"] = WiFi.SSID(i);
      docValueInfo["rssi"] = WiFi.RSSI(i);
      networksArray.add(docValueInfo);
      //Serial.print("Network name: ");
      //Serial.println(aWiFi.SSID(i));
      //Serial.print("Signal strength: ");
      //Serial.println(WiFi.RSSI(i));
      //Serial.println("-----------------------");
      
  }
  String stccall = "";
  serializeJson(doc, stccall);
  for (int i=0; i < this->totalAppender; i++) {
    callbacks[i].function(stccall); 
  }
}