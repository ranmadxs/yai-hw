#ifndef METRICS_H
#define METRICS_H

#include <Arduino.h>
#include "YaiWIFI.h"
#include "YaiGrafana.h"
#include "YaiTime.h"

class Metrics {
public:
    // Cambiamos YaiWIFI para ser un puntero
    Metrics(YaiWIFI* yaiWifi, const char* apiKey);  
    void sendCountMetric(const String& metricName, float count, const String& service, const String& host);
    void setOffsetTime(long offsetTime);

private:
    const char* apiKey;
    YaiWIFI* yaiWifi;  // Ahora es un puntero a YaiWIFI
    long offsetTime = 0;
    const String endpoint = "https://api.datadoghq.com/api/v1/series?api_key=";
};

#endif
