#ifndef YaiLog_h
#define YaiLog_h

#include <Arduino.h>
#include <logger.h>

#define TOTAL_LOG_CALLBACKS 10

// Definiciones de los niveles de log
const u_int16_t LOG_LEVEL_INFO = 1;
const u_int16_t LOG_LEVEL_ERROR = 2;
const u_int16_t LOG_LEVEL_DEBUG = 3;
const u_int16_t LOG_LEVEL_WARN = 4;
const u_int16_t LOG_LEVEL_VERBOSE = 5;

// Definiciones de colores ANSI para la consola serial
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

class LogAppender {
  public:
    ~LogAppender() {}
    LogAppender(void (*f)(String, String, String, const char*, int) = 0)
        : function(f) {}
    void (*function)(String, String, String, const char*, int);
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

  protected:
    String yrname;
    LogAppender callbacks[TOTAL_LOG_CALLBACKS];
    int totalAppender;

  private:
    void baseLog(String str, int level, const char* file, int line);
};

#endif
