#include <TinyGPS++.h>
#include <HardwareSerial.h>

// Configuration des broches RX et TX pour la communication avec le module GPS
#define GPS_RX_PIN 16 // GPIO16 (RX2)
#define GPS_TX_PIN 17 // GPIO17 (TX2)

// Initialisation de l'objet HardwareSerial pour communiquer avec le module GPS
HardwareSerial gpsSerial(2); // Utilisation du port série matériel 2 (RX2, TX2)

// Initialisation de l'objet TinyGPSPlus pour le traitement des données GPS
// Define serial ports for GPS and debugging

// Create a TinyGPS++ object
TinyGPSPlus gps;

void setup()
{
  // Initialize serial ports
  Serial.begin(9600);
  gpsSerial.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
}

void loop()
{
  // Read GPS data from the GPS module
  while (gpsSerial.available() > 0)
  {
    if (gps.encode(gpsSerial.read()))
    {
      // Check the validity of each data field
      if (gps.speed.isValid()) {
        Serial.print("Speed (km/h): ");
        Serial.println(gps.speed.kmph());
      } else {
        Serial.println("Speed (km/h): NULL");
      }
      
      if (gps.altitude.isValid()) {
        Serial.print("Altitude (m): ");
        Serial.println(gps.altitude.meters());
      } else {
        Serial.println("Altitude (m):NULL");
      }
      
      if (gps.location.isValid()) {
        Serial.print("Latitude: ");
        Serial.println(gps.location.lat(), 6);
        Serial.print("Longitude: ");
        Serial.println(gps.location.lng(), 6);
      } else {
        Serial.println("Latitude: NULL");
        Serial.println("Longitude: NULL");
      }
      
      if (gps.date.isValid()) {
        Serial.print("Date: ");
        Serial.print(gps.date.year());
        Serial.print("-");
        Serial.print(gps.date.month());
        Serial.print("-");
        Serial.println(gps.date.day());
        
      } else {
        Serial.println("Date : NULL");

      }
      
      if (gps.time.isValid()) {
        Serial.print("Time: ");
        Serial.print(gps.time.hour());
        Serial.print(":");
        Serial.print(gps.time.minute());
        Serial.print(":");
        Serial.println(gps.time.second());
      } else {
        Serial.println("Time: NULL");
      }
      
      Serial.println();
    }
  }
}
