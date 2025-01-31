#include <Arduino.h>
#include "YaiCommons.h"
//#include "Metrics.h"
#include "YaiWIFI.h"
#include <PubSubClient.h>
#include "YaiMqtt.h"
//#include "OledDisplay.h"
#include "KeypadHandler.h"

//const char* YAI_VERSION="0.0.20-SNAPSHOT";
// WiFiClient espClient;
unsigned long contadorWifi = 0;
void serialController();
void keyController();
void btnController();
unsigned long CONTATOR_TOTAL = 150000;
KeypadHandler keypadHandler;

// Crear instancias de las clases
// OledDisplay oledDisplay;
const char* datadogApiKey = "";
//Metrics metrics(&yaiWifi, datadogApiKey, MQTT_CLIENT_ID, yaiWifi.getIp());

void setup() {

  Serial.begin(115200); // opens serial port, sets data rate to 115200 bps
  existCMD = false;
  isBtnActive = false;
  //oledDisplay.iniciarPantalla();
  Serial.println(" ####################################");
  String yaiServerVersion = " ## tomi-yai-commander v" + String(YAI_VERSION) + " ##";
  Serial.println(yaiServerVersion);
  LOG_DEBUG(logger, yaiServerVersion);  // Usamos la macro LOG_DEBUG para capturar archivo y línea
  Serial.println(" ####################################");  
  all_init();
  all_off();
  // Por defecto dejamos prendido el relay 6
  digitalWrite(NODEMCU_ARRAY_PINS[5], RelayOn);
  if (ENABLE_WIFI) { 
    Serial.println(" ######### Wifi Client ##########");
    yaiWifi.connect();
  }
  if (ENABLE_WIFI) { 
    Serial.println(" ######### DNS Server ###########");
    String dnsName = "YAI_SRV_RELAYS";
    LOG_INFO(logger, dnsName);  // Usamos la macro LOG_INFO
    yaiWifi.startDNSServer(dnsName);
  }
  if (ENABLE_MQTT) { 
    clientMqtt.setServer(MQTT_SERVER, MQTT_PORT);
    clientMqtt.setCallback(callbackMqtt);
  }

  /* Init Button */
  //pinMode(ButtonPin,INPUT);
  // Configuración del pin del botón como entrada
  //pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void loop() {

  contadorWifi++;
  if (contadorWifi >= CONTATOR_TOTAL/2) {
    yaiWifi.loop();
  }  
  //btnController();
  serialController();
  keyController();
  logger.loop();
  if (ENABLE_MQTT) { 
    if (!clientMqtt.connected()) {
      reconnect();
    }
    clientMqtt.loop();
  }
}

// Función para el controlador del teclado antiguo
void keyController_old(){
  char tecla = keypadHandler.obtenerTecla();
  if (tecla) {
    // Mostrar en el puerto serial
    LOG_DEBUG(logger, "Apretaste el botón: ");
    LOG_DEBUG(logger, String(tecla));
    // Mostrar en la pantalla OLED
    //oledDisplay.mostrarTecla(tecla);
  }  
}

// Control del teclado para alternar relés con teclas del 1 al 8
void keyController() {
  char tecla = keypadHandler.obtenerTecla();
  if (tecla) {
    // Mostrar en el puerto serial
    LOG_DEBUG(logger, "Apretaste el botón: ");
    LOG_DEBUG(logger, String(tecla));
    // Mostrar en la pantalla OLED
    //oledDisplay.mostrarTecla(tecla);
  } 
  if (tecla >= '1' && tecla <= '8') { // Acepta teclas del 1 al 8
    int relayNumber = tecla - '0'; // Convierte la tecla a número (char '1' -> int 1)

    // Crear un comando para alternar el relé correspondiente
    YaiCommand yaiCommand;
    yaiCommand.type = "BTN";
    int currentState = digitalRead(NODEMCU_ARRAY_PINS[relayNumber - 1]);
    yaiCommand.command = currentState == RelayOn ? "OFF" : "ON";
    yaiCommand.p1 = String(relayNumber);  // Relé correspondiente del 1 al 8
    yaiCommand.execute = EXECUTE_CMD;
    yaiCommand.print = PRINT_CMD;

    // Ejecutar el comando a través de la función commandFactoryExecute
    commandFactoryExecute(yaiCommand);
  }
}

// Función para controlar el botón físico
void btnController() {
  if (digitalRead(BUTTON_PIN) == 1) {
    isBtnActive = isBtnActive ? false : true;
    YaiCommand yaiCommand;
    yaiCommand.type = "BTN";
    yaiCommand.command = isBtnActive ? "ON" : "OFF";
    yaiCommand.execute = EXECUTE_CMD;
    yaiCommand.print = PRINT_CMD;
    commandFactoryExecute(yaiCommand);
    delay(500);
  }
}

// Controlador para manejar comandos desde el puerto serial
void serialController() {
	YaiCommand yaiResCmd;
	YaiCommand yaiCommand;
	yaiCommand = yaiUtil.commandSerialFilter();
  if (String(YAI_COMMAND_TYPE_SERIAL) == yaiCommand.type) {
    // Serial.println(yaiCommand.toString());
    yaiCommand.execute = EXECUTE_CMD;
    yaiCommand.print = PRINT_CMD;
    commandFactoryExecute(yaiCommand);
  }	
	// if(yaiResCmd.type == String(YAI_COMMAND_TYPE_RESULT)){
		// Serial.print(">> ");
		// Serial.println(yaiResCmd.json);
	// }
}
