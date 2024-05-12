# ESP32-connected-GPS-Datalogger
This project is a GPS datalogger based on esp32
It feature :
* A battery gestion system
* A GPS datalogger
* A communication an ESP32 and mobile App

## Hardware minimal Requirements
* ESP32
* Any type of GPS that send NMEA trams, (we use TinyGPS++ to read data), as a reference i used Benitian BP220
* SD Card

## 1.1 (11/05/2024)
Datalogger update :
* Avrage Speed, duration, positive altitude change, total distence
* Every second gps data writing on sd card can be disabled

Added : Communication file that includes
* ESP32 Bluetooth Classic program that can send : File by segement of 128 octets, Files in a directory, File info (size, number of divisible packets...) 
* Python test that can recieve simple bluetooth data (supports hello world command form the blueooth classic program, issue with aquisition of large string)
If this solution doesn't work it wil be a ESP 32 will be an HTTP Server on Wifi AP that can send JSON file to the mobile device

## 1.0 First Relsase (16/04/2024)
This project include a GPS datalogger that write data (Date, hour, speed,  and the battery gestion system for now

Patch to do : Comments are in french -> Change to english
