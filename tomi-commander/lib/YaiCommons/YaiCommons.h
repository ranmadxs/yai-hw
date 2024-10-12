#ifndef YaiCommons_h
#define YaiCommons_h

#include <Arduino.h>
#include "YaiWIFI.h"
#include "YaiLog.h"  // Incluir el archivo de cabecera YaiLog para utilizar el logger
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

const char* MQTT_SERVER = "broker.mqttdashboard.com";
const u_int16_t MQTT_PORT = 1883; //1883
const char* MQTT_USER = "test";
const char* MQTT_PASSWORD = "test";

class YaiCommand;
void commandFactoryExecute(YaiCommand yaiCommand);
const bool ENABLE_WIFI = true;
const bool ENABLE_HTTP_SRV = true;
const bool ENABLE_YAI_PUMP_HEIGHT = false;
const bool ENABLE_MQTT = true && ENABLE_WIFI;
const bool ENABLE_WEBSOCKETS = true && ENABLE_WIFI;
const char* YAI_UID_NAME = "TYC01";

#define EXECUTE_CMD     true
#define RelayOn         LOW
#define RelayOff        HIGH
#define PRINT_CMD       true
#define BUTTON_PIN 4  // Pin GPIO4 (D2 en la placa NodeMCU)
bool isBtnActive;
String serialCMD;
bool existCMD;

typedef enum {
	PIN_D0 = 16,
    PIN_D1 = 5,
    PIN_D2 = 4,
    PIN_D3 = 0,
    PIN_D4 = 2,
    PIN_D5 = 14,
    PIN_D6 = 12,
    PIN_D7 = 13,
    PIN_D8 = 15,
	PIN_SD3 = 10,
	PIN_SD2 = 9,
	PIN_RX = 3
} NodeMCUPins;

NodeMCUPins NODEMCU_OLD_ARRAY_PINS[] = {PIN_D0, PIN_D1, PIN_D2, PIN_D3, PIN_D5, PIN_D6, PIN_D7, PIN_SD3};

typedef enum {
	PIN_32_D23 = 23,
    PIN_32_D22 = 22,
    PIN_32_TX0 = 1,
    PIN_32_RX0 = 3,
    PIN_32_D21 = 21,
    PIN_32_D19 = 19,
    PIN_32_D18 = 18,
    PIN_32_D5 = 5,
    PIN_32_TX2 = 17,
	PIN_32_RX2 = 16,
	PIN_32_D4 = 4,
	PIN_32_D2 = 2,
	PIN_32_D15 = 15,

	PIN_32_VP = 36,
	PIN_32_VN = 39,
	PIN_32_D34 = 34,
	PIN_32_D35 = 35,
	PIN_32_D32 = 32,
	PIN_32_D33 = 33
} NodeMCU32Pins;

NodeMCU32Pins NODEMCU_ARRAY_PINS[] = {
										PIN_32_D23, PIN_32_D22, PIN_32_D21, PIN_32_D19, 
										PIN_32_D18, PIN_32_D5, PIN_32_D4, PIN_32_D2
									};

YaiWIFI yaiWifi;
PubSubClient clientMqtt(yaiWifi.espClient);

// Inicialización del logger
YaiLog logger(YAI_UID_NAME);

void mqttCallback(String msg) {
  clientMqtt.publish(MQTT_TOPIC_OUT, msg.c_str());
}

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
		p8 = String(NULL_STR_VALUE);
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
	String p8;
	String json;
	boolean execute;
	boolean print;
	boolean propagate;
	int address;
  String error;

	String toString(){
    return (type+","+command+","+p1+","+p2+","+p3+","+p4+","+p5+","+p6+","+p7+","+p8);
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
			serialIn = Serial.readString();
			if (serialIn.length() > 0) {
				yaiCommand.message = serialIn;
				yaiCommand.type = String(YAI_COMMAND_TYPE_SERIAL);
				string2YaiCommand(yaiCommand);
			}
		}
		return yaiCommand;
	}

	void string2YaiCommand(YaiCommand &yaiCommand) {
		yaiCommand.command = "";
		yaiCommand.execute = false;
		if (yaiCommand.message != "") {
			yaiCommand.print = true;
			String root[8];
			getElementRoot(yaiCommand.message, root);
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
};

YaiUtil yaiUtil;

void all_init(){
  /* Init Relay */
  for (int i = 0; i < sizeof(NODEMCU_ARRAY_PINS)/sizeof(NODEMCU_ARRAY_PINS[0]); i++) {
    pinMode(NODEMCU_ARRAY_PINS[i], OUTPUT);
  }  
}

void all_off() {
  for (int i = 0; i < sizeof(NODEMCU_ARRAY_PINS)/sizeof(NODEMCU_ARRAY_PINS[0]); i++) {
    digitalWrite(NODEMCU_ARRAY_PINS[i], RelayOff);
  }
}

void all_on() {
  for (int i = 0; i < sizeof(NODEMCU_ARRAY_PINS)/sizeof(NODEMCU_ARRAY_PINS[0]); i++) {
    digitalWrite(NODEMCU_ARRAY_PINS[i], RelayOn);
  }
}

void commandFactoryExecute(YaiCommand yaiCommand) {
    YaiCommand yaiResCmd;
    Serial.print("<< ");
    Serial.println(yaiCommand.toString());

    if (yaiCommand.execute) {
        existCMD = false;

        if (yaiCommand.command == "ON") {
            Serial.println("POWER ON");
            existCMD = true;
            isBtnActive = true;

            bool algunoEncendido = false;
            int pins[] = {yaiCommand.p1.toInt(), yaiCommand.p2.toInt(), yaiCommand.p3.toInt(),
                          yaiCommand.p4.toInt(), yaiCommand.p5.toInt(), yaiCommand.p6.toInt(), 
                          yaiCommand.p7.toInt(), yaiCommand.p8.toInt()};

            for (int i = 0; i < 8; i++) {
                if (pins[i] > 0) {
                    digitalWrite(NODEMCU_ARRAY_PINS[pins[i] - 1], RelayOn);
                    algunoEncendido = true;
                }
            }

            if (!algunoEncendido) {
                all_on();
            }
        }

        if (yaiCommand.command == "OFF") {
            existCMD = true;
            isBtnActive = false;
            Serial.println("POWER OFF");

            bool algunoApagado = false;
            int pins[] = {yaiCommand.p1.toInt(), yaiCommand.p2.toInt(), yaiCommand.p3.toInt(),
                          yaiCommand.p4.toInt(), yaiCommand.p5.toInt(), yaiCommand.p6.toInt(), 
                          yaiCommand.p7.toInt(), yaiCommand.p8.toInt()};

            for (int i = 0; i < 8; i++) {
                if (pins[i] > 0) {
                    digitalWrite(NODEMCU_ARRAY_PINS[pins[i] - 1], RelayOff);
                    algunoApagado = true;
                }
            }

            if (!algunoApagado) {
                all_off();
            }
        }

        if (!existCMD) {
            yaiCommand.error = yaiCommand.command + " command not found";
        }

    } else {
        Serial.println("[WARN] Not execute command " + yaiCommand.command);
    }

    if (yaiCommand.error.length() > 1) {
        Serial.println("[ERROR] " + yaiCommand.error);
    }
}

#endif
