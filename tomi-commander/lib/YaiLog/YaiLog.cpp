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

// Función auxiliar para convertir el nivel de log a string
String getLogLevelString(int level) {
    switch(level) {
        case LOG_LEVEL_INFO:
            return "INFO";
        case LOG_LEVEL_ERROR:
            return "ERROR";
        case LOG_LEVEL_DEBUG:
            return "DEBUG";
        case LOG_LEVEL_WARN:
            return "WARN";
        case LOG_LEVEL_VERBOSE:
            return "VERBOSE";
        default:
            return "UNKNOWN";
    }
}

void serialAppender(String yrname, String msg, String levelName, const char* file, int line) {
    String timestamp = getFormattedTime(); // Llamada a la función para obtener la hora
    Serial.print("["+levelName+ "] ");


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
        callbacks[i].function(yrname, str, getLogLevelString(level), file, line); // Pasa el nombre, archivo, línea y nivel al appender
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
