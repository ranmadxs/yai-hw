
#include <Arduino.h>
#include <LittleFS.h>
#include <PubSubClient.h>
#include "YaiWIFI.h"
#include "YaiHttpSrv.h"

char message_buff[100];

const String YAI_UID = "WP01";

const char* mqtt_server = "broker.hivemq.com";
//const char* mqtt_server = "192.168.1.40";
const char* mqtt_user = "test";
const char* mqtt_password = "test";

void httpController();
void handleNotFound();
void callback(char* topic, byte* payload, unsigned int length);

//AsyncWebServer server(80);

// WiFiClient espClient;
YaiWIFI yaiWifi;
PubSubClient client(yaiWifi.espClient);


class CustomHttpController : public YaiHttpSrv {
  public:
    void httpController() override {
      getServer()->on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "YaiHttpSrv Ready, must implement22 YaiHttpSrv::httpController");
      });
    }
};

CustomHttpController yaiHttpSrv;

void setup(void) {
	Serial.begin(9600);	
	Serial.println("");
	Serial.println(" ###############################");
	Serial.println(" ## YaiServer v0.0.1-SNAPSHOT ##");
	Serial.println(" ###############################");


  FSInfo fs_info;
  if (LittleFS.begin()) {
		Serial.println("LittleFS ready");
	}
  LittleFS.info(fs_info);
  Serial.println(fs_info.totalBytes);
  Serial.println(fs_info.usedBytes);
  Serial.println(fs_info.maxOpenFiles);
  Serial.println(fs_info.maxPathLength);
  
	Serial.println(" ######### Wifi Client ##########");
  yaiWifi.connect();
	Serial.println(" ######### DNS Server ###########");
  yaiWifi.startDNSServer("YAI_SRV_" + YAI_UID);
	Serial.println(" ######### HTTP Server ##########");
  yaiHttpSrv.start();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void reconnect() {
  if (!client.connected()) {
    client.connect("ESP8266Client");    
    client.subscribe("inYaiTopic");
  }
}

void loop(void) {
  reconnect();
  yaiWifi.dnsServer.processNextRequest();
  client.loop();
}


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  int i = 0;
  for(i=0; i<length; i++) {
    message_buff[i] = payload[i];
  }
  message_buff[i] = '\0';  
  String msgString = String(message_buff);
  Serial.println("Payload: " + msgString);
}

String processor(const String& var){
  Serial.print("Processor: ");
  Serial.println(var);
  if (var == "YAI_IP")
    return "YAI_IP Value";
  if (var == "YAI_UID")
    return "YAI_UID Value";
}

void test(){
  Dir dir = LittleFS.openDir("/html");
  Serial.println("-----------InHtml-------------");
  while (dir.next()) {
    Serial.print(dir.fileName());
    if(dir.fileSize()) {
        File f = dir.openFile("r");
        Serial.println(f.size());
    }
  }
  Serial.println("----------InDir /------------");
  Dir dir2 = LittleFS.openDir("/");
  while (dir2.next()) {
    Serial.print(dir2.fileName());
    if(dir2.fileSize()) {
        File f2 = dir2.openFile("r");
        Serial.println(f2.size());
    }
  }  

  Serial.println("------------------------------");

  File dataFile = LittleFS.open("/html/index.html", "r");
  
  while(dataFile.available()){
    Serial.write(dataFile.read());
  }
  Serial.println("------------------------------");
}