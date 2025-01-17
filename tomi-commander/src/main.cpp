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
unsigned long CONTATOR_TOTAL = 150000;
unsigned long contadorExterno = CONTATOR_TOTAL;
unsigned long contadorWifi = 0;
unsigned long LOG_INFO_COUNTER = 0;
unsigned long LOG_DEBUG_COUNTER = 0;

// Arreglo para almacenar el conteo de cada tecla presionada
//unsigned long keyPressCounts[16] = {0};

// Arreglo de caracteres que representan las teclas del keypad
char keyChars[16] = {'1','2','3','A','4','5','6','B','7','8','9','C','*','0','#','D'};

const char* datadogApiKey = "";
void serialController();
void keyController();
void mqttController();
void mqttControllerTask(void* param);

// Función para obtener el índice de la tecla en el arreglo keyChars
int getKeyIndex(char keyChar) {
    for (int i = 0; i < 16; i++) {
        if (keyChars[i] == keyChar) {
            return i;
        }
    }
    return -1; // Si la tecla no se encuentra
}

KeypadHandler keypadHandler;
Metrics metrics(&yaiWifi, datadogApiKey, MQTT_CLIENT_ID, yaiWifi.getIp());
YaiTime yaiTime;

void loggerMetricsAppender(String yrname, String msg, String level, const char* file, int line) {
  String levelStr = String(level);
  //metrics.setService(MQTT_CLIENT_ID);
  //metrics.setHost(yaiWifi.getIp());
  levelStr.toUpperCase();
  Serial.println(" loggerMetricsAppender::"+ levelStr);
  if (levelStr.equals("ERROR")) {
    metrics.sendCountMetric("yai.log." + levelStr + ".count", 1);  // Enviar métrica a Datadog
  }
  if (levelStr.equals("INFO")) {
    //LOG_INFO_COUNTER++;
  }
  if (levelStr.equals("DEBUG")) {
    //LOG_DEBUG_COUNTER++;
  }
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

  if (yaiWifi.isConnected()) { 
    metrics.setService(MQTT_CLIENT_ID);
    metrics.setHost(yaiWifi.getIp());   
    yaiTime.syncTimeWithNTP(&yaiWifi);
    Serial.println("Timestamp actual (Epoch): " + String(yaiTime.getCurrentEpoch()));
    metrics.setOffsetTime(yaiTime.getCurrentEpoch());

    logger.addAppender(LogAppender(loggerMetricsAppender));
    Serial.println(" ######### DNS Server ###########");
    String dnsName = "YAI_SRV_RELAYS";
    LOG_INFO(logger, dnsName);
    yaiWifi.startDNSServer(dnsName);
    metrics.sendCountMetric("yai.wifi.status.ok.count", 1);
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

void metricsController(){
  contadorExterno++;
  if (contadorExterno >= CONTATOR_TOTAL) {
    contadorExterno = 0;
    metrics.sendCountMetric("yai.tomi.commander.keepalive.count", 1);
    /*
    if(LOG_INFO_COUNTER > 0)
      metrics.sendCountMetric("yai.log.INFO.count", LOG_INFO_COUNTER);  // Enviar métrica a Datadog
    if(LOG_DEBUG_COUNTER > 0)
      metrics.sendCountMetric("yai.log.DEBUG.count", LOG_DEBUG_COUNTER);  // Enviar métrica a Datadog
    LOG_INFO_COUNTER = 0;
    LOG_DEBUG_COUNTER = 0;

    // Enviar los conteos acumulados de las teclas
    for (int i = 0; i < 16; i++) {
        if (keyPressCounts[i] > 0) {
            String keyName = String(keyChars[i]);
            metrics.sendCountMetric("yai.keycontroller." + keyName + ".count", keyPressCounts[i]);
            keyPressCounts[i] = 0; // Reiniciar el conteo después de enviar
        }
    }
    */
  }
}

void loop() {
  contadorWifi++;
  if (contadorWifi >= CONTATOR_TOTAL/2) {
    yaiWifi.loop();
  }
  serialController();
  keyController();
  metricsController();
  logger.loop();
}

void serialController() {
	YaiCommand yaiResCmd;
	YaiCommand yaiCommand;
	yaiCommand = yaiUtil.commandSerialFilter();
  if (String(YAI_COMMAND_TYPE_SERIAL) == yaiCommand.type) {
    metrics.sendCountMetric("yai.serialController.message.count", 1);
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

    // Incrementar el conteo de la tecla presionada
    int keyIndex = getKeyIndex(tecla);
    /*
    if (keyIndex >= 0) {
        keyPressCounts[keyIndex]++;
    }
    */
    // Eliminar la llamada directa a sendCountMetric
    // metrics.sendCountMetric("yai.keycontroller." + String(tecla) + ".count", 1);

    // Resto del código permanece igual
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
}
