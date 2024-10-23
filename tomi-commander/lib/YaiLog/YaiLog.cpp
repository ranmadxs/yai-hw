#include "YaiLog.h"

// Colores ANSI para la consola serial (funcionan en algunos terminales seriales)
#define ANSI_RESET  "\u001B[0m"
#define ANSI_BLUE   "\u001B[34m"
#define ANSI_RED    "\u001B[31m"
#define ANSI_YELLOW "\u001B[33m"
#define ANSI_GREEN  "\u001B[32m"

// Función para formatear el tiempo transcurrido desde el inicio (milisegundos) en HH:MM:SS
String getFormattedTime() {
    unsigned long currentMillis = millis();
    unsigned long totalSeconds = currentMillis / 1000;
    unsigned int hours = (totalSeconds / 3600) % 24;
    unsigned int minutes = (totalSeconds / 60) % 60;
    unsigned int seconds = totalSeconds % 60;

    char timeBuffer[9];  // Buffer para almacenar el tiempo formateado
    snprintf(timeBuffer, sizeof(timeBuffer), "%02d:%02d:%02d", hours, minutes, seconds);
    return String(timeBuffer);
}

// Implementación del serialAppender utilizando una instancia de Logger con formato estilo Java
void serialAppender(String yrname, String msg, int level) {
    String timestamp = getFormattedTime(); // Obtener la marca de tiempo formateada

    switch (level) {
        case LOG_LEVEL_INFO:
            Serial.print(ANSI_GREEN);
            Serial.print("[INFO] ");
            break;
        case LOG_LEVEL_ERROR:
            Serial.print(ANSI_RED);
            Serial.print("[ERROR] ");
            break;
        case LOG_LEVEL_DEBUG:
            Serial.print(ANSI_BLUE);
            Serial.print("[DEBUG] ");
            break;
        case LOG_LEVEL_WARN:
            Serial.print(ANSI_YELLOW);
            Serial.print("[WARN] ");
            break;
        default:
            Serial.print("[UNKNOWN LEVEL] ");
            break;
    }
    
    // Imprimir la marca de tiempo, el nombre del logger y el mensaje
    Serial.print(timestamp);
    Serial.print(" - ");
    Serial.print(yrname);
    Serial.print(": ");
    Serial.println(msg);
    
    // Resetear color
    Serial.print(ANSI_RESET);
}

YaiLog::YaiLog() : totalAppender(0) {
    addAppender(LogAppender(serialAppender));
}

YaiLog::YaiLog(String yr_name) : yrname(yr_name), totalAppender(0) {
    addAppender(LogAppender(serialAppender));
}

void YaiLog::addAppender(LogAppender lolaso) {
    callbacks[totalAppender] = lolaso;
    totalAppender++;
}

void YaiLog::baseLog(String str, int level) {
    for (int i = 0; i < totalAppender; i++) {
        callbacks[i].function(yrname, str, level); // Pasa el nombre y el nivel al appender
    }
}

void YaiLog::info(String str) {
    baseLog(str, LOG_LEVEL_INFO);
}

void YaiLog::error(String str) {
    baseLog(str, LOG_LEVEL_ERROR);
}

void YaiLog::debug(String str) {
    baseLog(str, LOG_LEVEL_DEBUG);
}

void YaiLog::warn(String str) {
    baseLog(str, LOG_LEVEL_WARN);
}
