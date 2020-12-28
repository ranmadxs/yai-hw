#ifndef YaiContext_h
#define YaiContext_h
#include <Arduino.h>
#include <LittleFS.h>
#include <PubSubClient.h>
#include "YaiWIFI.h"
#include "YaiCommons.hpp"

const char* YAI_UID_NAME = "WP01";

char message_buff[100];
YaiLog logger(YAI_UID_NAME);
//const String YAI_UID = "WP01";

const char* mqtt_server = "broker.hivemq.com";
//const char* mqtt_server = "192.168.1.40";
const char* mqtt_user = "test";
const char* mqtt_password = "test";

const bool ENABLE_WIFI = true;
const bool ENABLE_HTTP_SRV = true;
const bool ENABLE_MQTT = true && ENABLE_WIFI;

//AsyncWebServer server(80);

// WiFiClient espClient;
YaiWIFI yaiWifi;
PubSubClient clientMqtt(yaiWifi.espClient);
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];

#endif