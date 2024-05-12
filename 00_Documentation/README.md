
## Princpal Library used on this project (Datalogger)
* [TinyGPS++](https://github.com/mikalhart/TinyGPSPlus.git) -> To read GPS Data
* [ArduinoJson](https://github.com/bblanchon/ArduinoJson.git) -> To write & read in JSON format
* [Arduino-esp32](https://github.com/espressif/arduino-esp32.git) librairies (HardwareSerial, SD, FS, SPI) -> To interface with SD card
* [Liquid Crystal_I2C](https://github.com/johnrickman/LiquidCrystal_I2C.git) -> For LCD integration (Only used on Datalogger V4/V5/V6)

## Hardware used
*Important: This project operates on battery, so it requires battery management.*
* ESP32
* Any type of GPS that sends NMEA sentences. *For reference, I used Benitian BP220, other types of gps like NEO-6M should surely do the job*
* SD Card
* Common anode RGB LED
* Push Button
* 3-position switch
* I²C 20x4 LCD (connected to default I²C Ports) : for test on battery, to monitor data during tests, used on Datalogger V4/V5/V6
For cable rooting see [Shematic](00_Documentation/PCB_Connected_GPS_Datalogger_Project.pdf)
