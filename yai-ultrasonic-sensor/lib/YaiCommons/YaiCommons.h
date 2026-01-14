#ifndef YaiCommons_h
#define YaiCommons_h

#include <Arduino.h>
#include "YaiWIFI.h"
#include <PubSubClient.h>
#define YAI_LOG_FOLDER						"/logs"

#define YAI_COMMAND_TYPE_SERIAL		"SER"
#define YAI_COMMAND_TYPE_SPI			"SPI"
#define YAI_COMMAND_TYPE_WIFI			"WIF"
#define YAI_COMMAND_TYPE_RESULT		"RES"
#define YAI_COMMAND_TYPE_NONE			"NIL"
#define YAI_COMMAND_TYPE_I2C			"I2C"

#define STATUS_OK								  "OKO"
#define STATUS_NOK								"NOK"
#define NULL_STR_VALUE			      "NULL"

const char* MQTT_TOPIC_IN = "yai-mqtt/in"; //IN
const char* MQTT_TOPIC_ALL = "yai-mqtt/#"; //IN
const char* MQTT_TOPIC_OUT = "yai-mqtt/out";

const char* MQTT_SERVER = "broker.mqttdashboard.com";//"broker.hivemq.com";//
const u_int16_t MQTT_PORT = 1883; //1883
const char* MQTT_USER = "test";
const char* MQTT_PASSWORD = "test";

class YaiCommand;
void commandFactoryExecute(YaiCommand yaiCommand);
const bool ENABLE_WIFI = true;
const bool ENABLE_HTTP_SRV = true;
const bool ENABLE_MQTT = true && ENABLE_WIFI;
const bool ENABLE_WEBSOCKETS = true && ENABLE_WIFI;
const char* YAI_UID_NAME = "US01";

#define EXECUTE_CMD     true
#define PRINT_CMD       true

String serialCMD;
bool existCMD;

const int TOTAL_LOG_CALBACKS = 10;

YaiWIFI yaiWifi;
//YaiLog logger(YAI_UID_NAME);
PubSubClient clientMqtt(yaiWifi.espClient);

void mqttCallback(String msg) {
  clientMqtt.publish(MQTT_TOPIC_OUT, msg.c_str());
}

void logToMqtt(String message) {
  Serial.println(message);
  if (ENABLE_MQTT && clientMqtt.connected()) {
    clientMqtt.publish(MQTT_TOPIC_OUT, message.c_str());
  }
}

class LogAppender {
  public:
    ~LogAppender(){}
    LogAppender(void (*f)(String) = 0)
        : function(f) {}
    void (*function)(String);
};

class YaiLog {
  public:
    YaiLog(){};
    YaiLog(String yr_name);
    void info(String str);
    void error(String str);
    void debug(String str);
    void warn(String str);
    void addAppender(LogAppender lolaso);

  protected:
	  String yrname;
    //LinkedList<Lolaso> callbacks;
    LogAppender callbacks[TOTAL_LOG_CALBACKS];
    int totalAppender;
    //Elements vector;

  
  private:
  
    void baseLog(String str);
};

class YaiCommand {
public:
	YaiCommand() {
		execute = false;
		print = false;
		propagate = false;
		message = "";
		type = String(NULL_STR_VALUE);
		p1 = String(NULL_STR_VALUE);
		p2 = String(NULL_STR_VALUE);
		p3 = String(NULL_STR_VALUE);
		p4 = String(NULL_STR_VALUE);
		p5 = String(NULL_STR_VALUE);
		p6 = String(NULL_STR_VALUE);
		p7 = String(NULL_STR_VALUE);
		address = 0;
		json = "";
    error = "";
	}

	String message;
	String type;
	String command;
	String p1;
	String p2;
	String p3;
	String p4;
	String p5;
	String p6;
	String p7;
	String json;
	boolean execute;
	boolean print;
	boolean propagate;
	int address;
  String error;

	String toString(){
    return (type+","+command+","+p1+","+p2+","+p3+","+p4+","+p5+","+p6+","+p7);
	}
};

class YaiUtil {
public:
	YaiUtil() {
	}

	YaiCommand commandSerialFilter() {
		String serialIn;
		YaiCommand yaiCommand;
		yaiCommand.type = String(NULL_STR_VALUE);
		yaiCommand.message = "";
		yaiCommand.execute = false;
		yaiCommand.print = false;
		if (Serial.available() > 0) {
			//serialIn = Serial.readStringUntil('\n');
      serialIn = Serial.readString();
      // Serial.println(">> " + serialIn);
			if (serialIn.length() > 0) {
				yaiCommand.message = serialIn;
				yaiCommand.type = String(YAI_COMMAND_TYPE_SERIAL);
				string2YaiCommand(yaiCommand);
        // Serial.println("<< " + yaiCommand.type);
			}
		}
		return yaiCommand;
	}

	void string2YaiCommand(YaiCommand &yaiCommand) {
		yaiCommand.command = "";
		yaiCommand.execute = false;
		// yaiCommand.type = String(YAI_COMMAND_TYPE_NONE);
		if (yaiCommand.message != "") {
			yaiCommand.print = true;
			String root[8];
			getElementRoot(yaiCommand.message, root);
			// yaiCommand.type = root[0];
			yaiCommand.command = root[0];
			yaiCommand.p1 = root[1];
			yaiCommand.p2 = root[2];
			yaiCommand.p3 = root[3];
			yaiCommand.p4 = root[4];
			yaiCommand.p5 = root[5];
			yaiCommand.p6 = root[6];
			yaiCommand.p7 = root[7];
		}
	}

	char *strSplit(char *str, const char *delim, char **save) {
		char *res, *last;

		if (!save)
			return strtok(str, delim);
		if (!str && !(str = *save))
			return NULL;
		last = str + strlen(str);
		if ((*save = res = strtok(str, delim))) {
			*save += strlen(res);
			if (*save < last)
				(*save)++;
			else
				*save = NULL;
		}
		return res;
	}
	;

	void getElementRoot(String myString, String rootElement[]) {
		char copy[64];
		myString.toCharArray(copy, 64);
		char *p = copy;
		char *str;

		int i = 0;

		while ((str = strSplit(p, ",", &p)) != NULL) {
			rootElement[i] = str;
			i++;
		}
	}
	;
};

YaiUtil yaiUtil;

// Command factory - to be implemented with ultrasonic sensor commands
void commandFactoryExecute(YaiCommand yaiCommand) {
	// Commands will be implemented here for ultrasonic sensor
}

#endif

