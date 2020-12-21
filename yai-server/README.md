pio run -t upload
pio run --target buildfs --environment nodemcuv2 
pio run --target uploadfs --environment nodemcuv2

pio device monitor

https://arduino-esp8266.readthedocs.io/en/latest/filesystem.html
https://randomnerdtutorials.com/esp8266-web-server-spiffs-nodemcu/