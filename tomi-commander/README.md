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

## Serial commands
```bash

# encender todos
ON,0,0,0,0,0,0,0
#apagar todos
OFF,0,0,0,0,0,0,0

# encender sólo el primero
ON,1,0,0,0,0,0,0
```
