# Yai Server

## script para subir todo
```bash
./compile.sh
```

## comando para compilar y subir programa a nodemcu

```bash
pio run -t upload --environment nodemcuv2
```

# para compilar y subir archivos
```bash
pio run --target buildfs --environment nodemcuv2
pio run --target uploadfs --environment nodemcuv2
```
pio run -e nodemcuv2

# List devices
pio device list

# Connect to monitor

## default 9600
```bash
pio device monitor
pio device monitor -b 115200
```

```bash
git commit -am "XD" --no-verify
```
https://arduino-esp8266.readthedocs.io/en/latest/filesystem.html
https://randomnerdtutorials.com/esp8266-web-server-spiffs-nodemcu/

https://www.luisllamas.es/comunicar-una-pagina-web-con-websockets-en-el-esp8266/

//curl -X GET "http://192.168.1.38/cmd/async=false/?TYPE=GET&COMMAND=LOGS&P1=NULL&P2=NULL&P3=NULL&P4=NULL&P5=NULL&P6=NULL&P7=NULL"
