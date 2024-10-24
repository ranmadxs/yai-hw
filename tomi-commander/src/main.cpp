#include <Arduino.h>
#include "YaiCommons.h"
#include "Metrics.h"
#include "YaiWIFI.h"
#include <PubSubClient.h>
#include "YaiMqtt.h"
//#include "OledDisplay.h"
#include "KeypadHandler.h"
//#include "YaiGrafana.h"
#include "YaiTime.h"
#if defined(ESP8266)
  #include <Ticker.h>  // Ticker para asincronía en ESP8266
#elif defined(ESP32)
  #include <freertos/FreeRTOS.h>
  #include <freertos/task.h>  // Para crear tareas asíncronas en ESP32
#endif

const char* datadogApiKey = "77e599b6cdd39b065667e3d441634fa3";
void serialController();
void keyController();
void btnController();
void mqttController();
void mqttControllerTask(void* param);

KeypadHandler keypadHandler;
Metrics metrics(&yaiWifi, datadogApiKey, MQTT_CLIENT_ID, yaiWifi.getIp());
YaiTime yaiTime;

void loggerMetricsAppender(String yrname, String msg, String level, const char* file, int line) {
  String levelStr = String(level);
  metrics.setService(MQTT_CLIENT_ID);
  metrics.sendCountMetric("yai.log." + levelStr + ".count", 1);  // Enviar métrica a Datadog
}

#if defined(ESP8266)
Ticker mqttTicker;  // Ticker para manejar asincronía en ESP8266
#elif defined(ESP32)
TaskHandle_t mqttTaskHandle = NULL;  // Identificador para la tarea de FreeRTOS en ESP32
#endif

void setup() {
  Serial.begin(115200);

  existCMD = false;
  isBtnActive = false;
  Serial.println(" ####################################");
  String yaiServerVersion = " ## tomi-yai-commander v" + String(YAI_VERSION) + " ##";
  Serial.println(yaiServerVersion);
  LOG_DEBUG(logger, yaiServerVersion);
  Serial.println(" ####################################");
  all_init();
  all_off();
  digitalWrite(NODEMCU_ARRAY_PINS[5], RelayOn);

  if (ENABLE_WIFI) { 
    Serial.println(" ######### Wifi Client ##########");
    yaiWifi.connect();
  }
  if (ENABLE_WIFI) { 
    yaiTime.syncTimeWithNTP(&yaiWifi);
    Serial.println("Timestamp actual (Epoch): " + String(yaiTime.getCurrentEpoch()));
    metrics.setOffsetTime(yaiTime.getCurrentEpoch());

    logger.addAppender(LogAppender(loggerMetricsAppender));
    Serial.println(" ######### DNS Server ###########");
    String dnsName = "YAI_SRV_RELAYS";
    LOG_INFO(logger, dnsName);
    yaiWifi.startDNSServer(dnsName);
  }
  if (ENABLE_MQTT) { 
    clientMqtt.setServer(MQTT_SERVER, MQTT_PORT);
    clientMqtt.setCallback(callbackMqtt);
  }

#if defined(ESP8266)
  // Asincronía para ESP8266 usando Ticker
  mqttTicker.attach(0.1, mqttController);  // Ejecuta mqttController cada 100 ms
#elif defined(ESP32)
  // Asincronía para ESP32 usando FreeRTOS
  xTaskCreatePinnedToCore(
    mqttControllerTask,  // Función de la tarea
    "MQTTControllerTask",  // Nombre de la tarea
    16384,  // Tamaño del stack
    NULL,  // Parámetros (ninguno en este caso)
    1,  // Prioridad de la tarea
    &mqttTaskHandle,  // Identificador de la tarea
    1  // Asignar al núcleo 1
  );
#endif
}

#if defined(ESP32)
// Función de la tarea para ESP32 (sin lambdas)
void mqttControllerTask(void* param) {
  for (;;) {
    mqttController();
    vTaskDelay(100 / portTICK_PERIOD_MS);  // Espera 100 ms entre ejecuciones
  }
}
#endif

void mqttController(){
  if (ENABLE_MQTT) { 
    if (!clientMqtt.connected()) {
      reconnect();
    }
    clientMqtt.loop();
  }
}

void loop() {
  serialController();
  keyController();
}

void serialController() {
	YaiCommand yaiResCmd;
	YaiCommand yaiCommand;
	yaiCommand = yaiUtil.commandSerialFilter();
  if (String(YAI_COMMAND_TYPE_SERIAL) == yaiCommand.type) {
    yaiCommand.execute = EXECUTE_CMD;
    yaiCommand.print = PRINT_CMD;
    commandFactoryExecute(yaiCommand);
  }
}

void keyController() {
  char tecla = keypadHandler.obtenerTecla();
  if (tecla) {
    LOG_DEBUG(logger, "Apretaste el botón: ");
    LOG_DEBUG(logger, String(tecla));
  }
  if (tecla >= '1' && tecla <= '8') {
    int relayNumber = tecla - '0';

    YaiCommand yaiCommand;
    yaiCommand.type = "BTN";
    int currentState = digitalRead(NODEMCU_ARRAY_PINS[relayNumber - 1]);
    yaiCommand.command = currentState == RelayOn ? "OFF" : "ON";
    yaiCommand.p1 = String(relayNumber);
    yaiCommand.execute = EXECUTE_CMD;
    yaiCommand.print = PRINT_CMD;

    commandFactoryExecute(yaiCommand);
  }
}
