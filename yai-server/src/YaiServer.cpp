
#include <Arduino.h>
#include <LittleFS.h>
#include <PubSubClient.h>
#include "YaiWIFI.h"
//#include "YaiPubSub.h"
#include "YaiWaterPumpController.h"

char message_buff[100];

const String YAI_UID = "WP01";

const char* mqtt_server = "broker.hivemq.com";
//const char* mqtt_server = "192.168.1.40";
const char* mqtt_user = "test";
const char* mqtt_password = "test";

const bool ENABLE_WIFI = true;
const bool ENABLE_HTTP_SRV = true;

void testLittleFS();
void reconnect();
void callback(char* topic, byte* payload, unsigned int length);

//AsyncWebServer server(80);

// WiFiClient espClient;
YaiWIFI yaiWifi;
PubSubClient client(yaiWifi.espClient);
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];

YaiWaterPumpController yaiHttpSrv;

void setup(void) {
	Serial.begin(115200);	
	Serial.println("");
	Serial.println(" ###############################");
	Serial.println(" ## YaiServer v0.0.1-SNAPSHOT ##");
	Serial.println(" ###############################");

  FSInfo fs_info;
  if (LittleFS.begin()) {
		Serial.println("LittleFS ready");
	}
  LittleFS.info(fs_info);

  testLittleFS();
  
  if (ENABLE_WIFI) { 
    Serial.println(" ######### Wifi Client ##########");
    yaiWifi.connect();
  }
  if (ENABLE_WIFI) { 
    Serial.println(" ######### DNS Server ###########");
    yaiWifi.startDNSServer("YAI_SRV_" + YAI_UID);
  }
	
  if (ENABLE_HTTP_SRV) { 
    Serial.println(" ######### HTTP Server ##########");
    yaiHttpSrv.start();
  }

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop(void) {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inYaiTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

}

void testLittleFS(){

  Serial.println("----------InDir /------------");
  Dir dir2 = LittleFS.openDir("/");
  while (dir2.next()) {
    Serial.print(dir2.fileName());
    if(dir2.fileSize()) {
        File f2 = dir2.openFile("r");
        Serial.println("  " + String(f2.size()) + "   " + f2.getCreationTime());
    } else {
      Serial.println("  <Dir>");
    }
  }  

  Dir dir = LittleFS.openDir("/html");
  Serial.println("-----------/html-------------");
  while (dir.next()) {
    Serial.print(dir.fileName());
    if(dir.fileSize()) {
        File f = dir.openFile("r");
        Serial.println(f.size());
    }
  }


  Serial.println("----------[/html/index.html]-------------");

  File dataFile = LittleFS.open("/html/index.html", "r");
  
  while(dataFile.available()){
    Serial.write(dataFile.read());
  }
  Serial.println("------------------------------");
}