#include "Metrics.h"

#if defined(ESP8266)
  #include <ESP8266HTTPClient.h>
#elif defined(ESP32)
  #include <HTTPClient.h>
#else
  #error "Plataforma no soportada"
#endif
#include <WiFiClientSecure.h>

// Constructor que recibe punteros a YaiWIFI, YaiGrafana y un apiKey opcional
Metrics::Metrics(YaiWIFI* yaiWifi, const char* apiKey) {
    this->apiKey = apiKey;  // Puede ser nullptr
    this->yaiWifi = yaiWifi;  // Asignamos el puntero a YaiWIFI
}

void Metrics::setOffsetTime(long offsetTime){
    this->offsetTime = offsetTime;
}

void Metrics::sendCountMetric(const String& metricName, float count, const String& service, const String& host) {
    if (this->yaiWifi && this->yaiWifi->isConnected()) {  // Verificar conexión Wi-Fi usando YaiWIFI
        
        // Crear la URL para la API de métricas de Datadog
        String url = this->endpoint + String(this->apiKey);
        
        // Obtener el timestamp actual
        unsigned long timestamp = millis() / 1000;
        timestamp = timestamp + this->offsetTime;
        // Crear el payload en formato JSON para enviar la métrica
        String payload = "{";
        payload += "\"series\": [{";
        payload += "\"metric\": \"" + metricName + "\",";
    //    payload += "\"points\": [[1729741143, " + String(count) + "]],";
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

    } else {
        Serial.println("No hay conexión Wi-Fi.");
    }
}
