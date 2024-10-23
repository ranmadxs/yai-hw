#ifndef YaiLog_h
#define YaiLog_h

#include <Arduino.h>
#include <logger.h>

#define TOTAL_LOG_CALBACKS 10

// Definiciones de los niveles de log
const u_int16_t LOG_LEVEL_INFO = 1;
const u_int16_t LOG_LEVEL_ERROR = 2;
const u_int16_t LOG_LEVEL_DEBUG = 3;
const u_int16_t LOG_LEVEL_WARN = 4;
const u_int16_t LOG_LEVEL_VERBOSE = 5;

class LogAppender {
  public:
    ~LogAppender(){}
    LogAppender(void (*f)(String, String, int) = 0)
        : function(f) {}
    void (*function)(String, String, int);
};

class YaiLog {
  public:
    YaiLog();
    YaiLog(String yr_name);
    void info(String str);
    void error(String str);
    void debug(String str);
    void warn(String str);
    void addAppender(LogAppender lolaso);

  protected:
    String yrname;
    LogAppender callbacks[TOTAL_LOG_CALBACKS];
    int totalAppender;
  
  private:
    void baseLog(String str, int level);
};

#endif
