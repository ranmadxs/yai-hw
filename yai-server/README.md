pio run -t upload
pio run --target buildfs --environment nodemcuv2 
pio run --target uploadfs --environment nodemcuv2

pio device monitor
pio device monitor -b 115200

https://arduino-esp8266.readthedocs.io/en/latest/filesystem.html
https://randomnerdtutorials.com/esp8266-web-server-spiffs-nodemcu/



//curl -X GET "http://192.168.1.38/cmd/async=false/?TYPE=GET&COMMAND=LOGS&P1=NULL&P2=NULL&P3=NULL&P4=NULL&P5=NULL&P6=NULL&P7=NULL"