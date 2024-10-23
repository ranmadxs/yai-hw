#include "YaiLog.h"

// Definición de la función para obtener el tiempo en formato HH:MM:SS
String getFormattedTime() {
    unsigned long currentMillis = millis();
    unsigned long totalSeconds = currentMillis / 1000;
    unsigned int hours = (totalSeconds / 3600) % 24;
    unsigned int minutes = (totalSeconds / 60) % 60;
    unsigned int seconds = totalSeconds % 60;

    char timeBuffer[9];  // Buffer para almacenar el tiempo formateado en HH:MM:SS
    snprintf(timeBuffer, sizeof(timeBuffer), "%02d:%02d:%02d", hours, minutes, seconds);
    return String(timeBuffer);  // Convertir a String y retornar
}

void serialAppender(String yrname, String msg, int level, const char* file, int line) {
    String timestamp = getFormattedTime(); // Llamada a la función para obtener la hora

    switch (level) {
        case LOG_LEVEL_INFO:
            Serial.print(ANSI_GREEN);  // Color verde para INFO
            Serial.print("[INFO] ");
            break;
        case LOG_LEVEL_ERROR:
            Serial.print(ANSI_RED);  // Color rojo para ERROR
            Serial.print("[ERROR] ");
            break;
        case LOG_LEVEL_DEBUG:
            Serial.print(ANSI_BLUE);  // Color azul para DEBUG
            Serial.print("[DEBUG] ");
            break;
        case LOG_LEVEL_WARN:
            Serial.print(ANSI_YELLOW);  // Color amarillo para WARN
            Serial.print("[WARN] ");
            break;
        default:
            Serial.print("[UNKNOWN LEVEL] ");
            break;
    }

    // Imprimir la marca de tiempo, el archivo, la línea, el nombre del logger y el mensaje
    Serial.print(timestamp);
    Serial.print(" - ");
    Serial.print(file);  // Mostrar el archivo
    Serial.print(":");
    Serial.print(line);  // Mostrar la línea
    Serial.print(" - ");
    Serial.print(yrname);
    Serial.print(": ");
    Serial.println(msg);

    // Resetear color
    Serial.print(ANSI_RESET);  // Resetear al color por defecto
}

YaiLog::YaiLog() : totalAppender(0) {
    addAppender(LogAppender(serialAppender));
}

YaiLog::YaiLog(String yr_name) : yrname(yr_name), totalAppender(0) {
    addAppender(LogAppender(serialAppender));
}

void YaiLog::addAppender(LogAppender appender) {
    callbacks[totalAppender] = appender;
    totalAppender++;
}

void YaiLog::baseLog(String str, int level, const char* file, int line) {
    for (int i = 0; i < totalAppender; i++) {
        callbacks[i].function(yrname, str, level, file, line); // Pasa el nombre, archivo, línea y nivel al appender
    }
}

void YaiLog::info(String str, const char* file, int line) {
    baseLog(str, LOG_LEVEL_INFO, file, line);
}

void YaiLog::error(String str, const char* file, int line) {
    baseLog(str, LOG_LEVEL_ERROR, file, line);
}

void YaiLog::debug(String str, const char* file, int line) {
    baseLog(str, LOG_LEVEL_DEBUG, file, line);
}

void YaiLog::warn(String str, const char* file, int line) {
    baseLog(str, LOG_LEVEL_WARN, file, line);
}
