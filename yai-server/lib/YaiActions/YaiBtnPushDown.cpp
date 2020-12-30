#include "YaiActions.hpp"

void startPinMode(int _pinBtn){
  pinMode(_pinBtn, INPUT);
}

YaiBtnPushDown::YaiBtnPushDown(int _pinBtn) {
  this->pinBtn = _pinBtn;
  this->buttonRead = 0;
}

void YaiBtnPushDown::setup(){
  startPinMode(this->pinBtn);
}

void YaiBtnPushDown::loop() {
  this->buttonRead=digitalRead(this->pinBtn);
  this->callback.function(this->buttonRead);
}

void YaiBtnPushDown::addCallback(void (*func)(int)) {
  this->callback = ActionAppender(func); 
}
