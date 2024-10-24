#ifndef YAITIME_H
#define YAITIME_H

#include <WiFiUdp.h>
#include <NTPClient.h>
#include "YaiWIFI.h"

class YaiTime {
private:
    WiFiUDP ntpUDP;
    NTPClient timeClient;

public:
    YaiTime();
    void syncTimeWithNTP(YaiWIFI* yaiWifi);
    unsigned long getCurrentEpoch();  // Método para obtener el timestamp en segundos
    String getFormattedTime();
};

#endif
