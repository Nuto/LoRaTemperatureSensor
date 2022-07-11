# LDTS - LoRa Distributed Temperature Sensor
A distributed Temperature Sensor connected via LoRa with a Master Module with a WLAN connection.

The aim of this project is to record the temperatures of the individual units in a small apartment complex in order to obtain an overall picture of the heating and cooling performance. The sensor data of the slave modules are then sent via LoRa to the master module. This then forwards the data via WLAN to a cloud service. This means that each slave module does not have to have its own WLAN configuration.

![Housing printed](doc/housing-printed.png)

## Used Hardware

- [1x Heltec - WIFI LoRa 32 - V2.1](https://amzn.to/3NMaJKi)
- [1x AZDelivery GY-BME280 Temperature Sensor](https://amzn.to/3Aph1wp)
- [1x Prototype Board](https://amzn.to/3OXw8Bc)

## Used Software Packages

- [sandeepmistry - arduino-LoRa](https://github.com/sandeepmistry/arduino-LoRa)
- [adafruit - Adafruit_BME280](https://github.com/adafruit/Adafruit_BME280_Library)
- [adafruit - Adafruit_SSD1306](https://github.com/adafruit/Adafruit_SSD1306)
- [esp8266 - ESP8266HTTPClient](https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266HTTPClient)

## Housing

![Housing](doc/housing.png)
![Cover](doc/housing-cover.png)

## Sensor data targets

- https://www.datagekko.com
- https://iotplotter.com
- https://iotguru.live
- https://thingspeak.com
