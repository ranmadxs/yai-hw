#ifndef YaiLog_h
#define YaiLog_h

#include <Arduino.h>

#define TOTAL_LOG_CALBACKS 10

class LogAppender {
  public:
    ~LogAppender(){}
    LogAppender(void (*f)(String) = 0)
        : function(f) {}
    void (*function)(String);
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
    void baseLog(String str);
};

#endif
