#ifndef YaiCommons_h
#define YaiCommons_h

#include <Arduino.h>

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

#endif

