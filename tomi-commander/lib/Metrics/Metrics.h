#ifndef METRICS_H
#define METRICS_H

#include <Arduino.h>
#include "YaiWIFI.h"
#include "YaiTime.h"

#if defined(ESP32)
  #include <freertos/FreeRTOS.h>
  #include <freertos/task.h>      // Para crear, manejar y eliminar tareas en FreeRTOS
  #include <freertos/queue.h>     // Para manejar colas en FreeRTOS
#endif

class Metrics {
public:
    // Constructor que recibe un puntero a YaiWIFI y un apiKey opcional
    Metrics(YaiWIFI* yaiWifi, const char* apiKey);  
    
    // Sobrecarga del constructor para recibir host y service
    Metrics(YaiWIFI* yaiWifi, const char* apiKey, const String& host, const String& service);

    // Modificado para usar la variable interna de service
    void sendCountMetric(const String& metricName, float count);
    
    // Método que permite pasar host y service directamente
    void sendCountMetric(const String& metricName, float count, const String& service, const String& host);

    void setOffsetTime(long offsetTime);
    
    // Setter para la variable host
    void setHost(const String& newHost);
    
    // Setter para la variable service
    void setService(const String& newService);

private:
    const char* apiKey;
    YaiWIFI* yaiWifi;  // Ahora es un puntero a YaiWIFI
    long offsetTime = 0;
    //const String endpoint = "https://api.datadoghq.com/api/v1/series?api_key=77e599b6cdd39b065667e3d441634fa3";
    const String endpoint = "https://tomi-metric-collector-production.up.railway.app/metrics";
    
    String host;    // Nueva variable privada host
    String service; // Nueva variable privada service

    // Método privado para enviar la métrica a Datadog (sincrónico)
    void sendToDatadog(const String& metricName, float count, const String& service, const String& host, unsigned long timestamp);

    // Sobrecarga de sendToDatadog que utiliza las variables internas service y host
    void sendToDatadog(const String& metricName, float count, unsigned long timestamp);

    // Método privado para manejar el envío de métricas de forma asíncrona
    void sendToDatadogAsync(const String& metricName, float count, const String& service, const String& host, unsigned long timestamp);

    // Estructura para almacenar los parámetros de las métricas en la cola
    struct SendTaskParams {
        Metrics* instance;       // Mantener la instancia de Metrics
        char metricName[64];     // Cambiado a char[] para evitar problemas con FreeRTOS
        float count;
        char service[32];        // Cambiado a char[]
        char host[32];           // Cambiado a char[]
        unsigned long timestamp;
    };

    // Tarea para procesar las métricas en la cola (solo para ESP32)
    static void processMetricsTask(void* param);

#if defined(ESP32)
    // Declaración de la cola de métricas para ESP32
    static QueueHandle_t metricsQueue;
#endif
};

#endif
