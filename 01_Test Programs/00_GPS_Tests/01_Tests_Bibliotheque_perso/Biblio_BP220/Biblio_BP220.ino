#include <Arduino.h>
#include "BP220_GPS.h"

#define GPS_SERIAL_RX 16  // Connecter RX du GPS à la broche 16 de l'Arduino
#define GPS_SERIAL_TX 17  // Connecter TX du GPS à la broche 17 de l'Arduino

BP220_GPS gps(GPS_SERIAL_RX, GPS_SERIAL_TX); // Déclaration et initialisation d'une instance de la classe BP220_GPS

void setup() {
  Serial.begin(9600);
  gps.begin();
}

void loop() {
  // Lire une trame NMEA
  String nmea = gps.readNMEA("DEFAULT");

  // Afficher la trame NMEA
  Serial.println("Trame NMEA : " + nmea);

  // Afficher les valeurs GGA extraites
  Serial.println("GGA Data:");
  Serial.println("Altitude: " + gps.GGAget_Altitude());
  Serial.println("Time: " + gps.GGAget_Time());
  Serial.println("Latitude: " + gps.GGAget_Latitude());
  Serial.println("IndicLat: " + gps.GGAget_indicLat());
  Serial.println("Longitude: " + gps.GGAget_longitude());
  Serial.println("IndicLong: " + gps.GGAget_indicLng());
  Serial.println();
  // Afficher les valeurs RMC extraites
  Serial.println("RMC Data:");
  Serial.println("Speed: " + gps.RMCget_Speed());
  Serial.println("Date: " + gps.RMCget_Date());
  Serial.println("Time: " + gps.RMCget_Time());
  Serial.println("Latitude: " + gps.RMCget_Latitude());
  Serial.println("IndicLat: " + gps.RMCget_indicLat());
  Serial.println("Longitude: " + gps.RMCget_longitude());
  Serial.println("IndicLong: " + gps.RMCget_indicLng());
  Serial.println();
  // Attendre un certain temps avant de lire la prochaine trame NMEA
  delay(1000);
}
