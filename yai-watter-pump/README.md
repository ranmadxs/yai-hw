## Run

### Run 01SerialPrint

```bash

# Compile
platformio run

# Compile and upload
platformio run --target upload

# if msg error 
# [Errno 13] Permission denied: '/dev/ttyUSB0'

sudo chown esanchez:esanchez /dev/ttyUSB0

# see serial device monitor send to file out
pio device monitor -f log2file -f default
```