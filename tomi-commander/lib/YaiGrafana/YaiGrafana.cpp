#include "YaiGrafana.h"
#include <WiFiUdp.h>
#include <TimeLib.h>   // Incluimos la librería TimeLib para manejar el tiempo

#if defined(ESP8266)
  #include <ESP8266HTTPClient.h>
#elif defined(ESP32)
  #include <HTTPClient.h>
#else
  #error "Plataforma no soportada"
#endif

// Método para limpiar los caracteres de control de una cadena
String cleanString(const String& input) {
    String output = input;
    output.replace("\n", "");  // Elimina saltos de línea
    output.replace("\r", "");  // Elimina retornos de carro
    return output;
}

// Constructor que recibe el endpoint de Loki o usa el por defecto
YaiGrafana::YaiGrafana(YaiWIFI* yaiWifi, const char* lokiEndpoint) {
    if (lokiEndpoint != nullptr) {
        this->lokiEndpoint = lokiEndpoint;
    } else {
        this->lokiEndpoint = defaultLokiEndpoint;  // Usa el endpoint por defecto
    }
    this->yaiWifi = yaiWifi;  // Asignamos el puntero

    // Establecer la fecha y hora manualmente
    // Ejemplo: setTime(hora, minutos, segundos, día, mes, año)
    setTime(12, 34, 56, 23, 10, 2024);  // Establecer la fecha y hora: 12:34:56 23-Oct-2024
}

// Método para enviar logs a Grafana Loki
void YaiGrafana::sendLog(const String& logMessage) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(lokiEndpoint);  // Especifica el endpoint de Loki
        http.addHeader("Content-Type", "application/json");

        // Obtenemos el tiempo actual en segundos desde la época UNIX
        time_t now_ = now();  // Obtener el tiempo actual configurado
        unsigned long long timestamp_ns = (unsigned long long)now_ * 1000000000ULL;  // Convertimos a nanosegundos
        // Limpiamos la cadena del log antes de enviarla a Loki
        String cleanedLogMessage = cleanString(logMessage);
        // Cuerpo de la solicitud para Loki con el timestamp generado en nanosegundos
        String lokiPayload = "{\"streams\": [{\"stream\": {\"app\": \"esp32\"}, \"values\": [[\"" + String(timestamp_ns) + "\", \"" + logMessage + "\"]]}]}";

        // Hacer la petición POST
        int httpResponseCode = http.POST(lokiPayload);

        if (httpResponseCode > 0) {
            String response = http.getString();
            Serial.println("Log enviado a Grafana Loki con éxito. Respuesta: " + response);
        } else {
            Serial.println("Error al enviar el log a Grafana Loki. Código de error: " + String(httpResponseCode));
        }

        http.end();
    } else {
        Serial.println("No hay conexión Wi-Fi.");
    }
}
