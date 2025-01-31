#ifndef YaiLog_h
#define YaiLog_h

#include <Arduino.h>
#include <logger.h>
#include "YaiWIFI.h"
#if defined(ESP32)
  #include <freertos/FreeRTOS.h>
  #include <freertos/queue.h>
  #include <freertos/task.h>
#elif defined(ESP8266)
  #include <Ticker.h>
#endif

#define TOTAL_LOG_CALLBACKS 10

// Definición de los niveles de log
const uint16_t LOG_LEVEL_INFO = 1;
const uint16_t LOG_LEVEL_ERROR = 2;
const uint16_t LOG_LEVEL_DEBUG = 3;
const uint16_t LOG_LEVEL_WARN = 4;
const uint16_t LOG_LEVEL_VERBOSE = 5;

// Definiciones de colores ANSI para la consola serial (vacío)
#define ANSI_RESET  ""
#define ANSI_BLUE   ""
#define ANSI_RED    ""
#define ANSI_YELLOW ""
#define ANSI_GREEN  ""

// Macros para capturar el archivo y la línea de código
#define LOG_DEBUG(logger, msg) (logger).debug((msg), __FILE__, __LINE__)
#define LOG_INFO(logger, msg) (logger).info((msg), __FILE__, __LINE__)
#define LOG_ERROR(logger, msg) (logger).error((msg), __FILE__, __LINE__)
#define LOG_WARN(logger, msg) (logger).warn((msg), __FILE__, __LINE__)

// Definir si se envían logs a DataDog
#define SEND_LOG_TO_DATADOG 1

class YaiLog;  // Declaración adelantada

class LogAppender {
  public:
    ~LogAppender() {}
    LogAppender(void (*f)(String, String, String, const char*, int) = 0)
        : function(f), objectPtr(nullptr), memberFunction(nullptr) {}
    LogAppender(YaiLog* obj, void (YaiLog::*mf)(String, String, String, const char*, int))
        : function(nullptr), objectPtr(obj), memberFunction(mf) {}
    void call(String yrname, String msg, String levelName, const char* file, int line);
  private:
    void (*function)(String, String, String, const char*, int);
    YaiLog* objectPtr;
    void (YaiLog::*memberFunction)(String, String, String, const char*, int);
};

class YaiLog {
  public:
    YaiLog();
    YaiLog(String yr_name);
    void info(String str, const char* file, int line);
    void error(String str, const char* file, int line);
    void debug(String str, const char* file, int line);
    void warn(String str, const char* file, int line);
    void addAppender(LogAppender appender);
    void loop();
    String getLogLevelString(int level);
    // Nuevo método para enviar logs asíncronamente a DataDog
    void sendToDatadogAsync();

  protected:
    boolean SEND_LOG_TO_DATADOG_FLAG = SEND_LOG_TO_DATADOG;
    String yrname;
    LogAppender callbacks[TOTAL_LOG_CALLBACKS];
    int totalAppender;

    // Estructura para representar un mensaje de log
    struct LogMessage {
        String message;
        String level;
        String service;
        String ddsource;
        String hostname;
        String tags;
        String date;
    };

    // Métodos y miembros para el appender de DataDog
    void dataDogAppender(String yrname, String msg, String levelName, const char* file, int line);
    void sendDataDogLogs();
    void addLogMessage(String message, String level);
    static const int MAX_LOG_MESSAGES = 2;
    LogMessage logMessages[MAX_LOG_MESSAGES];
    int logMessageCount;
    unsigned long lastSendTime;

    // Nuevo: Método para enviar los logs desde una tarea o ticker
    void sendDataDogLogsAsync();

  private:
    void baseLog(String str, int level, const char* file, int line);

#if defined(ESP32)
    // Estructuras y variables para ESP32
    struct SendTaskParams {
        YaiLog* instance;
    };
    static QueueHandle_t sendQueue;
    static void sendTask(void* parameter);
#elif defined(ESP8266)
    // Instancia de Ticker para ESP8266
    Ticker sendMetricTicker;
    void sendToDatadog();
#endif
};

#endif  // YaiLog_h