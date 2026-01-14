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
Distancia: 25.50 cm | Estado: OKO | Tiempo: 12345 ms
MQTT >> <DEVICE_ID>,OKO,25.50,12345
```

### Respuestas a comandos:
```
<DEVICE_ID>,Ultrasonic logs enabled, interval: 2000ms
<DEVICE_ID>,Ultrasonic logs disabled
<DEVICE_ID>,Unknown command: INVALID
```

## MQTT Topics

### Tópicos de comunicación:
- **IN**: `yai-mqtt/in` - Para comandos entrantes (control del sensor)
- **OUT General**: `yai-mqtt/out` - Para mensajes salientes del sensor (datos de distancia)
- **OUT Específico**: `yai-mqtt/<DEVICE_ID>/out` - Canal dedicado a este dispositivo

### Servidor MQTT por defecto:
- **Host**: `broker.mqttdashboard.com`
- **Puerto**: `1883`
- **Usuario/Contraseña**: `test/test`

## Formato de Mensajes MQTT

### Mensajes salientes (sensor → MQTT OUT):

Los mensajes se publican en **canales específicos según el tipo**:

#### Datos del sensor:
Los datos del sensor se envían **SOLO** al canal específico del dispositivo:
- `yai-mqtt/<DEVICE_ID>/out` - Canal específico del dispositivo

**Formato:**
```
<DEVICE_ID>,OKO,25.50,12345
```

**Campos:**
- `<DEVICE_ID>`: ID único del dispositivo (YUS = Ultrasonic Sensor + versión)
- `OKO`: Estado del sensor (`OKO` = OK, `NOK` = Error/Ningún objeto detectado)
- `25.50`: Distancia medida en centímetros (2 decimales)
- `12345`: Timestamp del sistema en milisegundos

#### Respuestas a comandos:
Las respuestas a comandos de control se envían a **AMBOS canales**:
- `yai-mqtt/out` - Canal general compartido
- `yai-mqtt/<DEVICE_ID>/out` - Canal específico del dispositivo

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

1. **Enviar comando para activar logs:**
   ```bash
   mosquitto_pub -h broker.mqttdashboard.com -t "yai-mqtt/in" -m "ON,2000,0,0,0,0,0,0"
   ```

2. **Recibir respuesta (en ambos canales):**
   ```
   yai-mqtt/out: <DEVICE_ID>,Ultrasonic logs enabled, interval: 2000ms
   yai-mqtt/<DEVICE_ID>/out: <DEVICE_ID>,Ultrasonic logs enabled, interval: 2000ms
   ```

3. **Recibir datos del sensor cada 2 segundos (solo canal específico):**
   ```
   yai-mqtt/<DEVICE_ID>/out: <DEVICE_ID>,OKO,25.50,12345
   yai-mqtt/<DEVICE_ID>/out: <DEVICE_ID>,OKO,25.60,14345
   ```

4. **Enviar comando para desactivar logs:**
   ```bash
   mosquitto_pub -h broker.mqttdashboard.com -t "yai-mqtt/in" -m "OFF,0,0,0,0,0,0,0"
   ```

5. **Recibir respuesta de confirmación (en ambos canales):**
   ```
   yai-mqtt/out: <DEVICE_ID>,Ultrasonic logs disabled
   yai-mqtt/<DEVICE_ID>/out: <DEVICE_ID>,Ultrasonic logs disabled
   ```

### Suscripción a canales específicos:

**Suscribirse solo al canal general:**
```bash
mosquitto_sub -h broker.mqttdashboard.com -t "yai-mqtt/out"
```

**Suscribirse solo al canal específico del dispositivo:**
```bash
mosquitto_sub -h broker.mqttdashboard.com -t "yai-mqtt/<DEVICE_ID>/out"
```

**Suscribirse a todos los canales del dispositivo:**
```bash
mosquitto_sub -h broker.mqttdashboard.com -t "yai-mqtt/<DEVICE_ID>/#"
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

### Estados del Sensor

- `OKO`: Medición exitosa, objeto detectado en rango válido
- `NOK`: Error o ningún objeto detectado (distancia ≤ 0 o > 400cm)

