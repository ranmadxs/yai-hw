## Run

### Run 01SerialPrint

```bash

# Compile
platformio run

# Compile and upload
platformio run --target upload
pio run -t upload --environment nodemcuv2
pio run -t upload --environment esp32

# if msg error 
# [Errno 13] Permission denied: '/dev/ttyUSB0'

sudo chown esanchez:esanchez /dev/ttyUSB0

```

# Connect to monitor

## default 9600
```bash
pio device monitor
pio device monitor -b 115200

# see serial device monitor send to file out
pio device monitor -f log2file -f default

## List Devices

pio device list
```

## Comandos de Control

El sensor ultrasónico puede ser controlado mediante comandos enviados por serial o MQTT:

### Apagar logs:
```bash
OFF,0,0,0,0,0,0,0
```
**Respuesta (Serial + MQTT):** `<DEVICE_ID>,Ultrasonic logs disabled`

### Prender logs con intervalo por defecto (1.5 segundos):
```bash
ON,0,0,0,0,0,0,0
```
**Respuesta (Serial + MQTT):** `<DEVICE_ID>,Ultrasonic logs enabled, interval: 1500ms`

### Prender logs con intervalo personalizado (ej: 2 segundos):
```bash
ON,2000,0,0,0,0,0,0
```
**Respuesta (Serial + MQTT):** `<DEVICE_ID>,Ultrasonic logs enabled, interval: 2000ms`

### Comando PING/PONG (verificación de conectividad):
```bash
PING
```
**Respuesta (solo MQTT):** `PONG,<DEVICE_ID>,IP:<ip_address>,IN:<input_topic>,OUT:<output_topic>,DEVICE_OUT:<device_output_topic>,SERVER:<server>:<port>`

### Comando HELP (ayuda de uso):
```bash
HELP
```
**Dónde:** mismo listener que PING → publicar en `yai-mqtt/in`  
**Respuesta (solo MQTT):** se publica en el mismo canal que PONG (`yai-mqtt/out`) un mensaje de ayuda con emojis y formato tipo Slack (comandos, canales, formato de lecturas).

### Cambiar intervalo sin apagar logs (ej: 3 segundos):
```bash
ON,3000,0,0,0,0,0,0
```
**Respuesta (Serial + MQTT):** `<DEVICE_ID>,Ultrasonic logs enabled, interval: 3000ms`

## Salida por Serial

### Mensaje de inicio:
```
 ####################################
 ## yai-ultrasonic-sensor <DEVICE_ID> ##
 ####################################
Iniciando sensor AJ-SR04M...
WiFi connected to: [SSID]
IP address: 192.168.1.100
MQTT connected
```

### Lecturas del sensor (cuando logs están activados):
```
Distancia: 25.50 cm | Estado: OKO | Tiempo: 2024-01-15 14:30:25
MQTT >> <DEVICE_ID>,OKO,25.50,2024-01-15 14:30:25
```

### Respuestas a comandos:
```
<DEVICE_ID>,Ultrasonic logs enabled, interval: 2000ms
<DEVICE_ID>,Ultrasonic logs disabled
<DEVICE_ID>,Unknown command: INVALID
```

## MQTT Topics

### Tópicos de comunicación:
- **IN General**: `yai-mqtt/in` - Comandos PING (conectividad) y HELP (ayuda de uso)
- **IN Específico**: `yai-mqtt/<CHANNEL_ID>/in` - Para comandos ON/OFF (control del sensor)
- **OUT General**: `yai-mqtt/out` - Respuestas a comandos, PONG y HELP
- **OUT Específico**: `yai-mqtt/<CHANNEL_ID>/out` - Para datos del sensor

### Servidor MQTT por defecto:
- **Host**: `broker.mqttdashboard.com`
- **Puerto**: `1883`
- **Usuario/Contraseña**: `test/test`

## Formato de Mensajes MQTT

### Mensajes salientes (sensor → MQTT OUT):

Los mensajes se publican en **canales específicos según el tipo**:

#### Datos del sensor:
Los datos del sensor se envían **SOLO** al canal específico del dispositivo:
- `yai-mqtt/<CHANNEL_ID>/out` - Canal específico del dispositivo

**Formato (JSON):**
```json
{
  "deviceId": "YUS-0.3.4-COSTA",
  "status": "OKO",
  "distanceCm": 25.50,
  "timestamp": "2024-01-15 14:30:25",
  "tankDepthCm": 160.0,
  "remainingToFullCm": 134.50,
  "fillLevelPercent": 84.06,
  "litros": 4203,
  "levelBar": "[########--]"
}
```

**Campos:**
- `deviceId`: ID del dispositivo con versión (`YUS-` + versión, por ejemplo `YUS-0.3.4-COSTA`)
- `status`: Estado del sensor (`OKO` = OK, `NOK` = Error/Ningún objeto detectado)
- `distanceCm`: Distancia medida por el sensor en centímetros (2 decimales)
- `timestamp`: Timestamp del sistema (formato NTP cuando hay WiFi, milisegundos si no)
- `tankDepthCm`: Profundidad total considerada del tanque en centímetros (ej: 160.0 = 1.60 m)
- `remainingToFullCm`: Cuánto le falta para llenarse en centímetros (0 = lleno, profundidad completa)
- `fillLevelPercent`: Nivel de llenado en porcentaje (0–100)
- `litros`: Cantidad de litros en el tanque (basado en `TANK_CAPACITY_LITERS`)
- `levelBar`: Barra visual de nivel `[##########]` donde `#` = lleno, `-` = vacío (10 posiciones)

#### Respuestas a comandos:
Las respuestas a comandos de control se envían a **AMBOS canales**:
- `yai-mqtt/out` - Canal general compartido
- `yai-mqtt/<CHANNEL_ID>/out` - Canal específico del dispositivo

**Formatos:**
```
<DEVICE_ID>,Ultrasonic logs enabled, interval: 2000ms
<DEVICE_ID>,Ultrasonic logs disabled
```

### Mensajes entrantes (MQTT IN → sensor):
Los comandos de control se reciben en el mismo formato CSV:
```
ON,2000,0,0,0,0,0,0
```

**Comandos soportados:**
- `ON,intervalo`: Activar logs con intervalo personalizado en ms
- `OFF`: Desactivar logs del sensor

### Ejemplos de uso MQTT:

1. **Activar logs cada 2 segundos:**
   ```bash
   mosquitto_pub -h broker.mqttdashboard.com -t "yai-mqtt/in" -m "ON,2000,0,0,0,0,0,0"
   ```

2. **Desactivar logs:**
   ```bash
   mosquitto_pub -h broker.mqttdashboard.com -t "yai-mqtt/in" -m "OFF,0,0,0,0,0,0,0"
   ```

3. **Suscribirse a los datos del sensor:**
   ```bash
   mosquitto_sub -h broker.mqttdashboard.com -t "yai-mqtt/out"
   ```

### Ejemplo de conversación MQTT:

1. **Enviar comando para activar logs (canal específico):**
   ```bash
   mosquitto_pub -h broker.mqttdashboard.com -t "yai-mqtt/<CHANNEL_ID>/in" -m "ON,2000,0,0,0,0,0,0"
   ```

2. **Recibir respuesta (en ambos canales):**
   ```
   yai-mqtt/out: <DEVICE_ID>,Ultrasonic logs enabled, interval: 2000ms
   yai-mqtt/<CHANNEL_ID>/out: <DEVICE_ID>,Ultrasonic logs enabled, interval: 2000ms
   ```

3. **Recibir datos del sensor cada 2 segundos (solo canal específico):**
   ```
   yai-mqtt/<CHANNEL_ID>/out: <DEVICE_ID>,OKO,25.50,2024-01-15 14:30:25
   yai-mqtt/<CHANNEL_ID>/out: <DEVICE_ID>,OKO,25.60,2024-01-15 14:30:27
   ```

4. **Enviar comando para verificar conectividad o ayuda (canal general):**
   ```bash
   mosquitto_pub -h broker.mqttdashboard.com -t "yai-mqtt/in" -m "PING"
   mosquitto_pub -h broker.mqttdashboard.com -t "yai-mqtt/in" -m "HELP"
   ```

5. **Recibir respuesta PONG o HELP (canal general `yai-mqtt/out`):**
   ```
   yai-mqtt/out: PONG,<DEVICE_ID>,IP:192.168.1.100,IN:yai-mqtt/in,OUT:yai-mqtt/out,DEVICE_IN:yai-mqtt/<CHANNEL_ID>/in,DEVICE_OUT:yai-mqtt/<CHANNEL_ID>/out,SERVER:broker.mqttdashboard.com:1883
   ```

6. **Enviar comando para desactivar logs (canal específico):**
   ```bash
   mosquitto_pub -h broker.mqttdashboard.com -t "yai-mqtt/<CHANNEL_ID>/in" -m "OFF,0,0,0,0,0,0,0"
   ```

7. **Recibir respuesta de confirmación (en ambos canales):**
   ```
   yai-mqtt/out: <DEVICE_ID>,Ultrasonic logs disabled
   yai-mqtt/<CHANNEL_ID>/out: <DEVICE_ID>,Ultrasonic logs disabled
   ```

### Ejemplos de comandos usando canales específicos:

**Enviar comando ON (solo funciona en canal específico):**
```bash
mosquitto_pub -h broker.mqttdashboard.com -t "yai-mqtt/<CHANNEL_ID>/in" -m "ON,2000,0,0,0,0,0,0"
```

**Enviar comando OFF (solo funciona en canal específico):**
```bash
mosquitto_pub -h broker.mqttdashboard.com -t "yai-mqtt/<CHANNEL_ID>/in" -m "OFF,0,0,0,0,0,0,0"
```

**Enviar PING o HELP (solo en canal general):**
```bash
mosquitto_pub -h broker.mqttdashboard.com -t "yai-mqtt/in" -m "PING"
mosquitto_pub -h broker.mqttdashboard.com -t "yai-mqtt/in" -m "HELP"
```

**Nota:** Los comandos ON/OFF serán ignorados si se envían al canal general (`yai-mqtt/in`).
Los comandos PING y HELP serán ignorados si se envían al canal específico (`yai-mqtt/<CHANNEL_ID>/in`).

### Suscripción a canales específicos:

**Suscribirse solo al canal general:**
```bash
mosquitto_sub -h broker.mqttdashboard.com -t "yai-mqtt/out"
```

**Suscribirse solo al canal específico del dispositivo:**
```bash
mosquitto_sub -h broker.mqttdashboard.com -t "yai-mqtt/<CHANNEL_ID>/out"
```

**Suscribirse a todos los canales del dispositivo:**
```bash
mosquitto_sub -h broker.mqttdashboard.com -t "yai-mqtt/<CHANNEL_ID>/#"
```

## Herramientas de Prueba MQTT

### Cliente web MQTT:
- **HiveMQ WebSocket Client**: https://www.hivemq.com/demos/websocket-client/
- Configurar conexión a: `broker.mqttdashboard.com:1883`

### Cliente de línea de comandos:

**Publicar mensaje:**
```bash
mosquitto_pub -h broker.mqttdashboard.com -t "yai-mqtt/in" -m "ON,1500,0,0,0,0,0,0"
```

**Suscribirse a tópico:**
```bash
mosquitto_sub -h broker.mqttdashboard.com -t "yai-mqtt/out"
```

### Configuración del Sensor

- **Pines utilizados:**
  - Trigger (envío de pulso): GPIO 5
  - Echo (recepción): GPIO 18
- **Rango de medición:** 2-400 cm
- **Intervalo por defecto:** 1500ms (1.5 segundos)
- **Velocidad del sonido:** 0.0343 cm/μs

### Configuración del Tanque (main.cpp)

```cpp
// Profundidad del tanque en cm (distancia del sensor al fondo)
extern const float TANK_DEPTH_CM = 160.0;

// Capacidad del tanque en litros cuando está lleno
extern const float TANK_CAPACITY_LITERS = 5000.0;
```

- `TANK_DEPTH_CM`: Altura desde el sensor hasta el fondo del tanque (ej: 160 cm = 1.60 m)
- `TANK_CAPACITY_LITERS`: Capacidad máxima del tanque en litros (ej: 5000 L)

### Estados del Sensor

- `OKO`: Medición exitosa, objeto detectado en rango válido
- `NOK`: Error o ningún objeto detectado (distancia ≤ 0 o > 400cm)

