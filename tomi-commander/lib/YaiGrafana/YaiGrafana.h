#ifndef YAIGRAFANA_H
#define YAIGRAFANA_H

#include <Arduino.h>
#include "YaiWIFI.h"

class YaiGrafana {
public:
    // Constructor que permite especificar un endpoint o usar el por defecto
    YaiGrafana(YaiWIFI* yaiWifi, const char* lokiEndpoint = nullptr);
    
    // Método para enviar logs
    void sendLog(const String& logMessage);

private:
    YaiWIFI* yaiWifi;  // Ahora es un puntero a YaiWIFI
    const char* lokiEndpoint;  // Endpoint para Loki
    const char* defaultLokiEndpoint = "";  // Endpoint por defecto
};

#endif
