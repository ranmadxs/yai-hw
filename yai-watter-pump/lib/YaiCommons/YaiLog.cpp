#include "YaiCommons.h"

void serialAppender(String msg) {
  Serial.println( msg );
}

void YaiLog::addAppender(LogAppender lolaso) {  
  callbacks[totalAppender] = lolaso;
  totalAppender++;
/*
  callbacks[0] = lolaso;
  callbacks[1] = lolaso;
  callbacks[2] = lolaso;
  */
 //callbacks.add(lolaso);

}

YaiLog::YaiLog(String yr_name){
  yrname = yr_name;
  totalAppender = 0;
  addAppender(LogAppender(serialAppender));
}

void YaiLog::baseLog(String str) {
  String varStr = yrname + " " + str;
   //Serial.println(yrname + " " + str);
   int count = 0;
    for (int i=0; i < totalAppender; i++) {
      callbacks[i].function(varStr); 
    }   
   //callbacks[0].function(str);   
     
  //int six = sizeof(callbacks);
  //int numUsers (sizeof(callbacks)/sizeof(char *)) ;//array siz
  //Serial.println("JODERRRRRRRRRRRRRRRRRRRRRRRRR" + String(six));
  //Serial.println("SEZE XDDDDD =" + String(numUsers));   
  // for (size_t i = 0; i < 1; i++)
  //  callbacks[i].function(yrname + " " + str);   
}

void YaiLog::info(String str){
 baseLog("[INFO] " +str);
 
}

void YaiLog::error(String str){
  baseLog("[ERROR] " +str);
}

void YaiLog::debug(String str){
  baseLog("[DEBUG] " +str);
}

void YaiLog::warn(String str){
  baseLog("[WARN] " +str);
}

