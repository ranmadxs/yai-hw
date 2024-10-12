#include "YaiLog.h"

void serialAppender(String msg) {
  Serial.println(msg);
}

YaiLog::YaiLog() : totalAppender(0) {
    addAppender(LogAppender(serialAppender));
}

YaiLog::YaiLog(String yr_name) : yrname(yr_name), totalAppender(0) {
    addAppender(LogAppender(serialAppender));
}

void YaiLog::addAppender(LogAppender lolaso) {  
  callbacks[totalAppender] = lolaso;
  totalAppender++;
}

void YaiLog::baseLog(String str) {
  String varStr = yrname + " " + str;
  for (int i = 0; i < totalAppender; i++) {
      callbacks[i].function(varStr); 
  }
}

void YaiLog::info(String str){
  baseLog("[INFO] " + str);
}

void YaiLog::error(String str){
  baseLog("[ERROR] " + str);
}

void YaiLog::debug(String str){
  baseLog("[DEBUG] " + str);
}

void YaiLog::warn(String str){
  baseLog("[WARN] " + str);
}
