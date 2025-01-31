#include "YaiLog.h"
#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266HTTPClient.h>
#elif defined(ESP32)
  #include <WiFi.h>
  #include <HTTPClient.h>
#endif

// Inicialización de la cola para ESP32
#if defined(ESP32)
QueueHandle_t YaiLog::sendQueue = NULL;
#endif
void serialAppender(String yrname, String msg, String levelName, const char* file, int line);
String getFormattedTime();

// Implementación de LogAppender::call
void LogAppender::call(String yrname, String msg, String levelName, const char* file, int line) {
    if (function) {
        function(yrname, msg, levelName, file, line);
    } else if (objectPtr && memberFunction) {
        (objectPtr->*memberFunction)(yrname, msg, levelName, file, line);
    }
}

// Implementación de YaiLog

YaiLog::YaiLog() : yrname("default"), totalAppender(0), logMessageCount(0), lastSendTime(millis()) {
    addAppender(LogAppender(serialAppender));
#if SEND_LOG_TO_DATADOG
    addAppender(LogAppender(this, &YaiLog::dataDogAppender));
#endif
}

YaiLog::YaiLog(String yr_name) : yrname(yr_name), totalAppender(0), logMessageCount(0), lastSendTime(millis()) {
    addAppender(LogAppender(serialAppender));
#if SEND_LOG_TO_DATADOG
    addAppender(LogAppender(this, &YaiLog::dataDogAppender));
#endif
}

void YaiLog::addAppender(LogAppender appender) {
    if (totalAppender < TOTAL_LOG_CALLBACKS) {
        callbacks[totalAppender] = appender;
        totalAppender++;
    }
}

void YaiLog::baseLog(String str, int level, const char* file, int line) {
    String levelStr = getLogLevelString(level);
    for (int i = 0; i < totalAppender; i++) {
        callbacks[i].call(yrname, str, levelStr, file, line);
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

String getFormattedTime() {
    unsigned long currentMillis = millis();
    unsigned long totalSeconds = currentMillis / 1000;
    unsigned int hours = (totalSeconds / 3600) % 24;
    unsigned int minutes = (totalSeconds / 60) % 60;
    unsigned int seconds = totalSeconds % 60;

    char timeBuffer[9];
    snprintf(timeBuffer, sizeof(timeBuffer), "%02d:%02d:%02d", hours, minutes, seconds);
    return String(timeBuffer);
}

String YaiLog::getLogLevelString(int level) {
    switch(level) {
        case LOG_LEVEL_INFO:
            return "info";
        case LOG_LEVEL_ERROR:
            return "error";
        case LOG_LEVEL_DEBUG:
            return "debug";
        case LOG_LEVEL_WARN:
            return "warn";
        case LOG_LEVEL_VERBOSE:
            return "verbose";
        default:
            return "unknown";
    }
}

// Appender para consola serial
void serialAppender(String yrname, String msg, String levelName, const char* file, int line) {
    String timestamp = getFormattedTime();
    Serial.print("[" + levelName + "] ");
    Serial.print(timestamp);
    Serial.print("-");
    Serial.print(file);
    Serial.print(":");
    Serial.print(line);
    Serial.print("-");
    Serial.print(yrname);
    Serial.print(":");
    Serial.println(msg);
}

void YaiLog::dataDogAppender(String yrname, String msg, String levelName, const char* file, int line) {
    addLogMessage(msg, levelName);
}

void YaiLog::addLogMessage(String message, String level) {
    if (logMessageCount < MAX_LOG_MESSAGES) {
        LogMessage& log = logMessages[logMessageCount++];
        log.message = message;
        log.level = level;
        log.service = yrname;
        log.ddsource = "arduino";
        log.hostname = WiFi.localIP().toString();
        log.tags = "env:production";
        log.date = getFormattedTime();  // Usando el tiempo formateado para simplicidad
    }
}

void YaiLog::loop() {
    this->sendDataDogLogs();
}

// Implementación de sendDataDogLogs
void YaiLog::sendDataDogLogs() {
    unsigned long currentTime = millis();
    // Verificar si es hora de enviar los logs
    if ((currentTime - lastSendTime >= 3 * 60 * 1000) || (logMessageCount >= MAX_LOG_MESSAGES)) {
        sendToDatadogAsync();
        lastSendTime = currentTime;
    }
}

void YaiLog::sendToDatadogAsync() {
    if (!SEND_LOG_TO_DATADOG_FLAG) {
        return;
    }

#if defined(ESP32)
    // Para ESP32, enviar una tarea a la cola
    if (sendQueue == NULL) {
        sendQueue = xQueueCreate(10, sizeof(SendTaskParams));
        if (sendQueue == NULL) {
            Serial.println("YaiLog :]> Error creando la cola para sendToDatadogAsync");
            return;
        }
        // Crear la tarea de envío
        if (xTaskCreatePinnedToCore(sendTask, "SendTask", 4096, this, 1, NULL, 1) != pdPASS) {
            Serial.println("YaiLog :]> Error creando la tarea para sendToDatadogAsync");
            return;
        }
    }

    // Crear los parámetros de la tarea
    SendTaskParams taskParams;
    taskParams.instance = this;

    // Enviar los parámetros a la cola
    if (xQueueSend(sendQueue, &taskParams, portMAX_DELAY) != pdPASS) {
        Serial.println("YaiLog :]> Error: No se pudo añadir la tarea a la cola para sendToDatadogAsync.");
    }

#elif defined(ESP8266)

    // Para ESP8266, usar Ticker para llamar a sendDataDogLogs de forma asíncrona, pero no funciona muy bien! para esp8266
    // sendMetricTicker.once_ms(100, [this]() { this->sendDataDogLogsAsync(); });
    this->sendDataDogLogsAsync();
#endif
}

void YaiLog::sendDataDogLogsAsync() {
    if (logMessageCount == 0) {
        return;
    }

    HTTPClient http;
    WiFiClient client;
    http.begin(client, "https://tomi-metric-collector-production.up.railway.app/logs");    
    //http.begin("https://tomi-metric-collector-production.up.railway.app/logs");

    http.addHeader("Content-Type", "application/json");

    // Construir el payload JSON
    String payload = "{\"logs\":[";
    for (int i = 0; i < logMessageCount; i++) {
        LogMessage& log = logMessages[i];
        payload += "{";
        payload += "\"message\":\"" + log.message + "\",";
        payload += "\"level\":\"" + log.level + "\",";
        payload += "\"service\":\"" + log.service + "\",";
        payload += "\"ddsource\":\"" + log.ddsource + "\",";
        payload += "\"hostname\":\"" + log.hostname + "\",";
        //payload += "\"date\":\"" + log.date + "\",";
        payload += "\"tags\":[\"" + log.tags + "\"]";

        payload += "}";
        if (i < logMessageCount - 1) {
            payload += ",";
        }
    }
    payload += "]}";

    int httpResponseCode = http.POST(payload);

    if (httpResponseCode > 0) {
        String response = http.getString();
        // Manejar la respuesta si es necesario
        Serial.println("YaiLog :]> Send to Datadog: logMessageCount=" + String(logMessageCount));
    } else {
        // Manejar error
        Serial.println("YaiLog :]> Error al enviar logs a tomiCollector: " + String(httpResponseCode));
    }
    http.end();

    // Reiniciar el contador de mensajes después de enviar
    logMessageCount = 0;
}

#if defined(ESP32)
  // Implementación de la tarea de envío para ESP32
  void YaiLog::sendTask(void* parameter) {
      YaiLog* logger = (YaiLog*) parameter;
      while (1) {
          // Esperar a que haya una tarea en la cola
          SendTaskParams params;
          if (xQueueReceive(sendQueue, &params, portMAX_DELAY)) {
              if (params.instance) {
                  params.instance->sendDataDogLogsAsync();
              }
          }
          // Añadir un pequeño retraso para evitar el bloqueo
          vTaskDelay(10 / portTICK_PERIOD_MS);
      }
  }
#endif