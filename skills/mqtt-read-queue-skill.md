# Skill: lectura de cola MQTT

Este skill está pensado para usarlo con **Aider** (no depende de `.cursor/`).

## Objetivo

Conectarse al broker MQTT y escuchar los mensajes del topic:

- Host: `broker.mqttdashboard.com`
- Puerto: `1883`
- Usuario: `test`
- Password: `test`
- Topic: `yai-mqtt/YUS-0.2.8-COSTA/out`

## Instrucciones para el asistente (Aider)

Cuando el usuario pida *leer*, *escuchar* o *monitorizar* la cola / topic MQTT anterior, sigue estos pasos usando **mosquitto_sub**:

1. Verifica que el comando `mosquitto_sub` está disponible:

   ```bash
   mosquitto_sub -h
   ```

2. Suscríbete al topic con las credenciales proporcionadas:

   ```bash
   mosquitto_sub \
     -h broker.mqttdashboard.com \
     -p 1883 \
     -u test \
     -P test \
     -t "yai-mqtt/YUS-0.2.8-COSTA/out"
   ```

3. Mantén el proceso corriendo hasta que el usuario indique que quiere detener la escucha (normalmente con `Ctrl+C` en el terminal).

## Notas

- Este skill está especializado en el topic fijo `yai-mqtt/YUS-0.2.8-COSTA/out`.
- Si el usuario pide cambiar de topic, host o credenciales, ajusta los parámetros de `mosquitto_sub` (`-h`, `-p`, `-u`, `-P`, `-t`) antes de ejecutarlo.
- Si la conexión falla, muestra el mensaje de error de `mosquitto_sub` y explica al usuario que no se pudo establecer la conexión.


