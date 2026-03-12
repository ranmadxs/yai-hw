# Especificación: UDP Discovery para App (Cliente)

Documento para implementar el lado del **cliente** (app Android AIA Agent u otro) del protocolo de discovery usado por los microcontroladores YAI.

Referencia del lado microcontrolador: [UDP-DISCOVERY-MICROCONTROLLER-SPEC.md](./UDP-DISCOVERY-MICROCONTROLLER-SPEC.md)

---

## Resumen

La app debe descubrir dispositivos en la red local mediante UDP. Hay dos formas de recibir dispositivos:

1. **Enviar AIA-DISCOVER** y recibir respuestas unicast (cuando el móvil está en la red del router).
2. **Escuchar broadcasts proactivos** del ESP (cuando el móvil está conectado al AP del dispositivo, 192.168.50.x).

---

## Constantes

| Campo | Valor |
|-------|-------|
| Puerto UDP | `9999` |
| Mensaje discovery | `AIA-DISCOVER` |
| Broadcast discovery | `255.255.255.255:9999` |
| Broadcast proactivo ESP | `192.168.50.255:9999` |
| Topics | `yai-mqtt/out` (lecturas), `yai-mqtt/in` (comandos) |

---

## Requisitos de la app

### 1. Socket UDP

- **Bind al puerto 9999** (ej: `0.0.0.0:9999`) para poder recibir:
  - Respuestas unicast a AIA-DISCOVER
  - Broadcasts proactivos del ESP (192.168.50.255:9999)

### 2. Discovery activo

Enviar periódicamente o al iniciar:

```
AIA-DISCOVER
```

- Destino: `255.255.255.255:9999`
- Codificación: UTF-8
- Sin salto de línea al final

### 3. Recepción de dispositivos

La app recibe paquetes de dos orígenes:

| Origen | Cuándo | Formato |
|--------|--------|---------|
| Respuesta a AIA-DISCOVER | Móvil en red del router | JSON unicast al puerto de la app |
| Broadcast proactivo | Móvil conectado al AP del ESP (192.168.50.x) | JSON a 192.168.50.255:9999 |

**Formato JSON de discovery:**

```json
{
  "id": "YUS-0.2.8-COSTA",
  "device_id": "YUS-0.2.8-COSTA",
  "channel_id": "1A2B3C4D",
  "ip": "192.168.50.1",
  "version": "1.0.6",
  "mqtt_topic_in": "yai-mqtt/in",
  "mqtt_topic_out": "yai-mqtt/out",
  "name": "Estanque Costa",
  "type": "estanque"
}
```

| Campo | Obligatorio | Descripción |
|-------|-------------|-------------|
| `id` / `device_id` | Sí | Identificador único |
| `ip` | Sí | IP para conectar al dispositivo |
| `channel_id` | No | ID corto del canal (8 hex) |
| `version` | No | Versión del firmware |
| `mqtt_topic_in` | No | Canal de comandos |
| `mqtt_topic_out` | No | Canal de lecturas |
| `name` | No | Nombre legible |
| `type` | No | Tipo: estanque, sensor, etc. |

**Criterio para detectar discovery:** Si el paquete es JSON válido y contiene `device_id` (o `id`) e `ip`, tratarlo como dispositivo descubierto.

### 4. Lecturas (ESP → App)

Tras el discovery, el ESP envía lecturas al subscriber:

```json
{"topic":"yai-mqtt/out","payload":{"deviceId":"...","distanceCm":25.5,"fillLevelPercent":84.06,...}}
```

La app debe seguir escuchando en el mismo socket para recibir estos mensajes.

### 5. Comandos (App → ESP)

Para enviar comandos al dispositivo:

```json
{"topic":"yai-mqtt/in","payload":"ON,2000,0,0,0,0,0,0"}
{"topic":"yai-mqtt/in","payload":"OFF,0,0,0,0,0,0,0"}
```

- Destino: `(IP_ESP, 9999)`
- IP_ESP: la IP recibida en el JSON de discovery

### 6. Timeout del subscriber

Si no hay discovery en 5 minutos, el ESP deja de enviar lecturas. La app puede reenviar `AIA-DISCOVER` para reactivar.

---

## Flujo resumido

```
1. App: bind socket a 0.0.0.0:9999
2. App: enviar "AIA-DISCOVER" a 255.255.255.255:9999
3. App: en loop, recvfrom() en el socket
4. Si llega JSON con device_id e ip → añadir/actualizar dispositivo
5. Si llega {"topic":"yai-mqtt/out",...} → procesar lectura
6. Para enviar comando → sendto(IP_ESP, 9999, {"topic":"yai-mqtt/in","payload":"..."})
```

---

## Caso especial: móvil conectado al AP del ESP

Cuando el usuario conecta el móvil al WiFi del ESP (SSID tipo `1A2B3C4D_YUS-1.0.6-COSTA`):

- El ESP está en 192.168.50.1
- El móvil recibe IP en 192.168.50.x
- El ESP **no recibe bien** broadcast cuando es AP
- El ESP envía **broadcast proactivo** cada 2 s a 192.168.50.255:9999
- La app **debe escuchar en 9999** para recibir esos broadcasts
- Si la app solo envía AIA-DISCOVER y espera respuesta, no funcionará (el ESP no recibe el broadcast)
