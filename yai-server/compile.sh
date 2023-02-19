#!/bin/sh

pio run --target buildfs --environment nodemcuv2
pio run --target uploadfs --environment nodemcuv2
platformio run --target upload --environment nodemcuv2
