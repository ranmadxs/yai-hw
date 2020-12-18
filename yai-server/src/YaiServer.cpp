#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>

const byte DNS_PORT = 53;
IPAddress apLocalIp(192, 168, 50, 1);
IPAddress apSubnetMask(255, 255, 255, 0);
DNSServer dnsServer;
boolean connectedWifi = false;
const int totalWifi = 3;
const int retryWifi = 17;
const String YAI_UID = "YSRV_001";
#define apSsid "YaiDNSServer"

char* arrayWifi[totalWifi][2] = {
		{ "Cachantun*", "2119amto" },
		{ "VTR-YAI-5Ghz", "Pana8bc1108" },
		{ "GalaxyJ1", "1101000000" } };

void wifiConnect();
void startDNSServer();
void startHttpServer();
void httpController();
void handleNotFound();
ESP8266WebServer server(80);

void setup(void) {
	Serial.begin(9600);
	WiFi.mode(WIFI_AP_STA);
	Serial.println("");
	Serial.println(" ###############################");
	Serial.println(" ## YaiServer v0.0.1-SNAPSHOT ##");
	Serial.println(" ###############################");
	Serial.println(" ######### Wifi Client ##########");
  wifiConnect();
	Serial.println(" ######### DNS Server ###########");
  startDNSServer();
	Serial.println(" ######### HTTP Server ##########");
  startHttpServer();
}

void loop(void) {
  dnsServer.processNextRequest();
	server.handleClient();
}

/*
void handleRoot() {
	String htmlSrc = "<HTML><BODY>YaisServer v.0.0.1-SNAPSHOT ["+YAI_UID+"]</BODY></HTML>";
  Serial.println("La lesera");
	server.send(200, "text/html", htmlSrc);
}
*/
void handleRoot() {

}

void httpController() {
  //server.on("/", handleRoot);
  //server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
  //  request->send(SPIFFS, "/index.html", String(), false, processor);
  //});  
  Serial.println("... [GET] /");
	server.onNotFound(handleNotFound);

	server.begin();
	Serial.println("HTTP server started");  
}

void startHttpServer() {
  if (MDNS.begin("esp8266")) {
		Serial.println("MDNS responder started");
	}
	MDNS.addService("http", "tcp", 80);
    // Initialize SPIFFS
  if(!SPIFFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  httpController();
}

void startDNSServer() {
  WiFi.softAPConfig(apLocalIp, apLocalIp, apSubnetMask);
  WiFi.softAP(apSsid);
	dnsServer.setTTL(300);
	dnsServer.setErrorReplyCode(DNSReplyCode::ServerFailure);
	dnsServer.start(DNS_PORT, "yairover.ddns.com", apLocalIp);
	Serial.println("DNS Server OK ip:" + apLocalIp.toString() );
	Serial.println("ssid:" + String(apSsid));  
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

void handleNotFound() {
	String content_type = "text/plain";
	String message = "File Not Found\n\n";
	int codeStatus = 404;
	message += "URI: ";
	message += server.uri();
	message += "\nMethod: ";
	message += (server.method() == HTTP_GET) ? "GET" : "POST";
	message += "\nArguments: ";
	message += server.args();
	message += "\n";
	for (uint8_t i = 0; i < server.args(); i++) {
		message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
	}

	server.send(codeStatus, content_type, message);
}