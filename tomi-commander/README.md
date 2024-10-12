# Tomi-yai-commander

## Connect to monitor

### default 115200

```bash {"id":"01JA0BQYHGVW5VH7VZEP1Y7EE7"}
pio device monitor -b 9600
pio device monitor -b 115200

# see serial device monitor send to file out
pio device monitor -f log2file -f default

## List Devices

pio device list

#Clean caches
pio run --target clean

#install libs
pio lib install

# compilar
pio run -t upload --environment esp32
```

## Serial commands

```bash {"id":"01JA0BQYHGVW5VH7VZEQCCG27X"}

# encender todos
ON,0,0,0,0,0,0,0,0
#apagar todos
OFF,0,0,0,0,0,0,0,0

# encender sólo el primero
ON,1,0,0,0,0,0,0,0

# encender sólo el primero y el sexto
ON,1,6,0,0,0,0,0,0
```
