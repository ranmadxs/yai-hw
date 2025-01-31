#include "YaiTime.h"

YaiTime::YaiTime() : timeClient(ntpUDP, "pool.ntp.org", 0, 60000) {}

void YaiTime::syncTimeWithNTP(YaiWIFI* yaiWifi) {
    if (yaiWifi->isConnected()) {
        timeClient.begin();
        while (!timeClient.update()) {
            timeClient.forceUpdate();
        }

        Serial.println("Fecha y hora sincronizadas desde NTP");
        Serial.println("Timestamp actual (Epoch): " + String(timeClient.getEpochTime()));
    } else {
        Serial.println("Error: No hay conexión WiFi");
    }
}

unsigned long YaiTime::getCurrentEpoch() {
    // Retorna el tiempo actual en segundos desde el Epoch
    return timeClient.getEpochTime();
}

String YaiTime::getFormattedTime() {
    return timeClient.getFormattedTime();
}