#ifndef YaiActions_h
#define YaiActions_h

#include <Arduino.h>

class ActionAppender {
  public:
    ~ActionAppender(){}
    ActionAppender(void (*f)(int) = 0)
        : function(f) {}
    void (*function)(int);
};

class YaiBtnPushDown {
  public:

    YaiBtnPushDown(int _pinBtn);
    void setup();
    void addCallback(void (*f)(int));
    void loop();

  protected:
	  String yrname;
  private:
    int pinBtn;
    int buttonRead;
    ActionAppender callback;
};

#endif