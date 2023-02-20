#ifndef YaiTime_hpp
#define YaiTime_hpp
#include <Arduino.h>
#include <time.h>

    class YaiTime {
        public:
            YaiTime(){};
            void setup();
        
        private:
            void printLocalTime();
        
        protected:
            const char* ntpServer = "pool.ntp.org";
            const long  gmtOffset_sec = 3600;
            const int   daylightOffset_sec = 3600;

    };

#endif