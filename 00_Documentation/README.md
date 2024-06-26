# Documentation
This project has been devlopped on Arduino IDE 2.0 with the esp32-arduino sdk and compiler


This project operates on battery
## Main library used on this project for ESP32
* [TinyGPS++](https://github.com/mikalhart/TinyGPSPlus.git) -> To read GPS Data
* [ArduinoJson](https://github.com/bblanchon/ArduinoJson.git) -> To write & read in JSON format
* [Arduino-esp32](https://github.com/espressif/arduino-esp32.git) librairies (HardwareSerial, SD, FS, SPI, Bluetooh Serial, Wifi) -> To interface with SD card and communicate
* [Liquid Crystal_I2C](https://github.com/johnrickman/LiquidCrystal_I2C.git) -> For LCD integration (Only used on Datalogger V4/V5/V6)
* [AsyncTCP](https://github.com/dvarrel/AsyncTCP) -> Requiered for ESPAsyncWebServer
* [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer/tree/master) -> To Handle HTTP Server and sending data 

## Hardware used
* ESP32
* Any type of GPS that sends NMEA sentences. *For reference, I used Benitian BP220, other types of gps like NEO-6M should surely do the job*
* SD Card
* Common anode RGB LED
* Push Button
* 3-position switch
* I²C 20x4 LCD (connected to default I²C Ports) : for test on battery, to monitor data during tests, used on Datalogger V4/V5/V6


For cable rooting see [Shematic](https://github.com/TL-Gamma/Perform-Chandail/blob/5c7101901dbf19d851dd238090973781bca3cb34/00_Documentation/PCB_Connected_GPS_Datalogger_Project.pdf)
