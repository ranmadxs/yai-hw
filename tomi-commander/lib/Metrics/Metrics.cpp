#include "Metrics.h"

#if defined(ESP8266)
  #include <ESP8266HTTPClient.h>
  #include <Ticker.h>  // Para manejar la asincronía en ESP8266
#elif defined(ESP32)
  #include <HTTPClient.h>
  #include <freertos/FreeRTOS.h>    // Base de FreeRTOS
  #include <freertos/task.h>        // Para crear y manejar tareas en ESP32
#else
  #error "Plataforma no soportada"
#endif
#include <WiFiClientSecure.h>

#if defined(ESP8266)
Ticker sendMetricTicker;  // Ticker para manejar asincronía en ESP8266
#endif

// Constructor que recibe punteros a YaiWIFI y un apiKey opcional
Metrics::Metrics(YaiWIFI* yaiWifi, const char* apiKey) {
    this->apiKey = apiKey;  // Puede ser nullptr
    this->yaiWifi = yaiWifi;  // Asignamos el puntero a YaiWIFI
}

void Metrics::setOffsetTime(long offsetTime) {
    this->offsetTime = offsetTime;
}

void Metrics::sendCountMetric(const String& metricName, float count, const String& service, const String& host) {
    if (this->yaiWifi && this->yaiWifi->isConnected()) {  // Verificar conexión Wi-Fi usando YaiWIFI
        unsigned long timestamp = millis() / 1000 + this->offsetTime;

        // Llamar de forma asíncrona al método privado que envía la métrica a Datadog
        sendToDatadogAsync(metricName, count, service, host, timestamp);
    } else {
        Serial.println("No hay conexión Wi-Fi.");
    }
}

// Método privado para encapsular la lógica de envío de la métrica a Datadog (sincrónico)
void Metrics::sendToDatadog(const String& metricName, float count, const String& service, const String& host, unsigned long timestamp) {
    // Crear la URL para la API de métricas de Datadog
    String url = this->endpoint + String(this->apiKey);

    // Crear el payload en formato JSON para enviar la métrica
    String payload = "{";
    payload += "\"series\": [{";
    payload += "\"metric\": \"" + metricName + "\",";
    payload += "\"points\": [[" + String(timestamp) + ", " + String(count) + "]],";
    payload += "\"type\": \"count\",";
    payload += "\"host\": \"" + host + "\",";
    payload += "\"tags\": [\"service:" + service + "\"]";
    payload += "}]}";

    // Usamos WiFiClientSecure para HTTPS
    WiFiClientSecure client;
    client.setInsecure();  // No verificamos el certificado (para simplificar)

    HTTPClient http;
    http.begin(client, url);  // Comenzamos la conexión HTTP
    http.addHeader("Content-Type", "application/json");  // Agregamos el header de contenido JSON

    // Realizamos la solicitud POST
    int httpResponseCode = http.POST(payload);
    Serial.println(payload);
    Serial.println(timestamp);
    
    // Verificar el código de respuesta
    if (httpResponseCode > 0) {
        Serial.print("Respuesta HTTP: ");
        Serial.println(httpResponseCode);
        String response = http.getString();  // Obtener respuesta del servidor
        Serial.println("Respuesta del servidor: " + response);
    } else {
        Serial.print("Error en la solicitud HTTP: ");
        Serial.println(httpResponseCode);
    }

    // Finalizamos la conexión
    http.end();
}

// Método para manejar la lógica asíncrona de envío de métrica a Datadog
void Metrics::sendToDatadogAsync(const String& metricName, float count, const String& service, const String& host, unsigned long timestamp) {
#if defined(ESP32)
    // Crear una tarea FreeRTOS para ESP32
    xTaskCreatePinnedToCore(
        sendToDatadogTask,  // Función de la tarea
        "DatadogTask",      // Nombre de la tarea
        4096,               // Tamaño del stack en bytes
        new SendTaskParams{this, metricName, count, service, host, timestamp},  // Parámetros para la tarea
        1,                  // Prioridad de la tarea
        NULL,               // Identificador de la tarea
        1                   // Núcleo al que se asigna la tarea (Core 1)
    );
#elif defined(ESP8266)
    // Usar Ticker para ESP8266
    sendMetricTicker.once(0.1, std::bind(&Metrics::sendToDatadog, this, metricName, count, service, host, timestamp));  // Llamar al método de envío después de 100 ms
#endif
}

// Tarea asíncrona para manejar el envío de métricas a Datadog en ESP32
void Metrics::sendToDatadogTask(void* params) {
    // Convertir los parámetros
    SendTaskParams* taskParams = static_cast<SendTaskParams*>(params);
    
    // Llamar al método sincrónico para enviar la métrica
    taskParams->instance->sendToDatadog(taskParams->metricName, taskParams->count, taskParams->service, taskParams->host, taskParams->timestamp);

    // Liberar la memoria asignada a los parámetros
    delete taskParams;

    // Finalizar la tarea
    vTaskDelete(NULL);
}
