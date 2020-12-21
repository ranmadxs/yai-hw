#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>
#include <Arduino.h>
#include <DNSServer.h>
#include <LittleFS.h>
#include <PubSubClient.h>


const byte DNS_PORT = 53;
IPAddress apLocalIp(192, 168, 50, 1);
IPAddress apSubnetMask(255, 255, 255, 0);
DNSServer dnsServer;
boolean connectedWifi = false;
char message_buff[100];
const int totalWifi = 3;
const int retryWifi = 17;
const String YAI_UID = "YSRV_001";
#define apSsid "YaiDNSServer"

//const char* mqtt_server = "broker.hivemq.com";
const char* mqtt_server = "192.168.1.40";
const char* mqtt_user = "test";
const char* mqtt_password = "test";

char* arrayWifi[totalWifi][2] = {
		{ "Cachantun*", "2119amto" },
		{ "VTR-YAI-5Ghz", "Pana8bc1108" },
		{ "GalaxyJ1", "1101000000" } };

void wifiConnect();
void startDNSServer();
void startHttpServer();
void httpController();
void handleNotFound();
void callback(char* topic, byte* payload, unsigned int length);

AsyncWebServer server(80);
WiFiClient espClient;
PubSubClient client(espClient);

void setup(void) {
	Serial.begin(9600);
	WiFi.mode(WIFI_AP_STA);
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
  Serial.println("------------------");

  
	Serial.println(" ######### Wifi Client ##########");
  wifiConnect();
	Serial.println(" ######### DNS Server ###########");
  startDNSServer();
	Serial.println(" ######### HTTP Server ##########");
  startHttpServer();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
/*

  if (client.connect("ESP8266Client")) {
    Serial.println("connected");
    //out
    client.publish("outYaiTopic", "init");
    //in
    client.subscribe("inYaiTopic");
    client.publish("outYaiTopic", "init22");
  } else {
    Serial.println("Y_:Y");
  }
*/
}

void reconnect() {
      // clientID, username, MD5 encoded password
      client.connect("ESP8266Client");
      client.publish("outYaiTopic", "I'm alive! ESP3266");
      client.subscribe("inYaiTopic");
}

void loop(void) {
  if (!client.connected()) {
    Serial.print("Connecting to MQTT " + String(mqtt_server));
    client.connect("ESP8266Client");
    client.publish("outYaiTopic", "I'm alive! ESP3266");
    client.subscribe("inYaiTopic");
  }
  dnsServer.processNextRequest();
  client.loop();
}


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  /*
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
*/
  int i = 0;

  //Serial.println("Message arrived:  topic: " + String(topic));
  //Serial.println("Length: " + String(length,DEC));
  
  // create character buffer with ending null terminator (string)
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

void httpController() {

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/html/index.html", String(), false, processor);
  });

}

void startHttpServer() {

  httpController();

	server.begin();
	Serial.println("HTTP server started");    
}

void startDNSServer() {
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

void wifiConnect() {
  char* ssid;
	char* password;
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
				Serial.print(" Conectado!!!");
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
