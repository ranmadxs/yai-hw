#ifndef METRICS_H
#define METRICS_H

#include <Arduino.h>
#include "YaiWIFI.h"
#include "YaiGrafana.h"
#include "YaiTime.h"

#if defined(ESP32)
  #include <freertos/FreeRTOS.h>
  #include <freertos/task.h>
#endif

class Metrics {
public:
    // Constructor que recibe un puntero a YaiWIFI y un apiKey opcional
    Metrics(YaiWIFI* yaiWifi, const char* apiKey);  
    void sendCountMetric(const String& metricName, float count, const String& service, const String& host);
    void setOffsetTime(long offsetTime);

private:
    const char* apiKey;
    YaiWIFI* yaiWifi;  // Ahora es un puntero a YaiWIFI
    long offsetTime = 0;
    const String endpoint = "https://api.datadoghq.com/api/v1/series?api_key=";

    // Método privado para enviar la solicitud a Datadog
    void sendToDatadog(const String& metricName, float count, const String& service, const String& host, unsigned long timestamp);

    // Método privado para enviar la solicitud a Datadog de manera asíncrona
    void sendToDatadogAsync(const String& metricName, float count, const String& service, const String& host, unsigned long timestamp);

    // Estructura para pasar parámetros a la tarea asíncrona
    struct SendTaskParams {
        Metrics* instance;
        String metricName;
        float count;
        String service;
        String host;
        unsigned long timestamp;
    };

    // Tarea asíncrona para ESP32
    static void sendToDatadogTask(void* params);
};

#endif
