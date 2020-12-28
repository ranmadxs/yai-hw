#include "YaiCommons.hpp"


YaiCommand::YaiCommand() {
  execute = false;
	print = false;
	propagate = false;
	message = "";
	type = String(NULL_STR_VALUE);
  command = String(NULL_STR_VALUE);
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


String YaiCommand::toString(){
  return (type+","+command+","+p1+","+p2+","+p3+","+p4+","+p5+","+p6+","+p7);
}

YaiCommand YaiCommand::commandSerialFilter() {
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

void YaiCommand::string2YaiCommand(YaiCommand &yaiCommand) {
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

char *YaiCommand::strSplit(char *str, const char *delim, char **save) {
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

void YaiCommand::getElementRoot(String myString, String rootElement[]) {
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
