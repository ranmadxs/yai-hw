#ifndef YaiCommons_h
#define YaiCommons_h

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

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

const int TOTAL_LOG_CALBACKS = 10;

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
	
  YaiCommand();

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

	String toString();

	YaiCommand commandSerialFilter();

  protected:
	  void string2YaiCommand(YaiCommand &yaiCommand);
	  char *strSplit(char *str, const char *delim, char **save);
	  void getElementRoot(String myString, String rootElement[]);
};

/************** NODEMCU PINTOUT *********************/

/*
int ESP_D0 = 16;
int ESP_D1 = 5;  // SCL (I2C)
int ESP_D2 = 4;  // SDA (I2C)
int ESP_D3 = 0;
int ESP_D4 = 2;  // TXD1
int ESP_D5 = 14; // SCLK
int ESP_D6 = 12; // MISO -> RDX2
int ESP_D7 = 13; // MOSI -> TXD2
int ESP_D8 = 15; // CS
*/
#endif

