*Disclamer : This project is in curent work and is commented mostly in french*
# ESP32-connected-GPS-Datalogger

This project is a GPS datalogger based on esp32
It feature :
* A battery gestion system
* A GPS datalogger (GPS provides : Speed, altitude, latitude, longitude, latitude, hour, date)
* JSON data saving on SD
* A communication an ESP32 and mobile App
## Library Requirements for esp32 datalogger
* ArduinoJson.h
* TinyGPS++.h
* HardwareSerial.h
* SD.h
* FS.h
* SPI.h
* Liquid Crystal_I2C.h (Only for LCD Version of the datalogger)
## Hardware minimal Requirements
* ESP32
* Any type of GPS that send NMEA trams, (we use TinyGPS++ to read data). *As a reference i used Benitian BP220*
* SD Card
* RGB LED
* Push Button

## 1.1 (11/05/2024)
Datalogger update :
* Avrage Speed, duration, positive altitude change, total distence
* Every second gps data writing on sd card can be disabled

Added : Communication directory that includes
* ESP32 Bluetooth Classic program that can send : File by segement of 128 octets, Files in a directory, File info (size, number of divisible packets...) 
* Python test that can recieve simple bluetooth data (supports hello world command form the blueooth classic program, issue with aquisition of large string)
If this solution doesn't work it wil be a ESP 32 will be an HTTP Server on Wifi AP that can send JSON file to the mobile device

## 1.0 First Relsase (16/04/2024)
Added : Datalogger directory that comes with : Datalogger V1/V2/V3/V4/V5/V6 & V6_LCD
Added : Unitary tests directory
Note : Only V6 & V6_LCD featues full features stable
