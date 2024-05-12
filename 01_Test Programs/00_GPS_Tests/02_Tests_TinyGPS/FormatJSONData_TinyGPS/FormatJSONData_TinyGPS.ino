#include <ArduinoJson.h> // Inclure la bibliothèque ArduinoJSON
#include <TinyGPS++.h>
#include <HardwareSerial.h>

// Configuration des broches RX et TX pour la communication avec le module GPS
#define GPS_RX_PIN 16 // GPIO16 (RX2)
#define GPS_TX_PIN 17 // GPIO17 (TX2)

// Initialisation de l'objet HardwareSerial pour communiquer avec le module GPS
HardwareSerial gpsSerial(2); // Utilisation du port série matériel 2 (RX2, TX2)

// Initialisation de l'objet TinyGPSPlus pour le traitement des données GPS
TinyGPSPlus gps;

DynamicJsonDocument jsonDoc(512); // Créer un document JSON dynamique

void setup()
{
  // Initialisation de la communication série avec une vitesse de 9600 bauds
  Serial.begin(9600);
  gpsSerial.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  
  Serial.println("Reception des donnees GPS :");
}

void loop()
{
  // Lecture des données GPS depuis le module GPS
  while (gpsSerial.available() > 0)
  {
    if (gps.encode(gpsSerial.read()))
    {
      // Vérification de la validité de chaque donnée
      if (gps.speed.isValid()) {
        jsonDoc["vitesse"] = gps.speed.kmph();
      } else {
        jsonDoc["vitesse"] = nullptr;
      }
      
      if (gps.altitude.isValid()) {
        jsonDoc["altitude"] = gps.altitude.meters();
      } else {
        jsonDoc["altitude"] = nullptr;
      }
      
      if (gps.location.isValid()) {
        jsonDoc["latitude"] = gps.location.lat();
        jsonDoc["longitude"] = gps.location.lng();
      } else {
        jsonDoc["latitude"] = nullptr;
        jsonDoc["longitude"] = nullptr;
      }
      
      if (gps.date.isValid()) {
        char dateBuffer[20]; // Tampon pour stocker la date formatée
        snprintf(dateBuffer, sizeof(dateBuffer), "%04d-%02d-%02d", gps.date.year(), gps.date.month(), gps.date.day());
        jsonDoc["date"] = dateBuffer;
      } else {
        jsonDoc["date"] = nullptr;
      }
      
      if (gps.time.isValid()) {
        char timeBuffer[20]; // Tampon pour stocker l'heure formatée
        snprintf(timeBuffer, sizeof(timeBuffer), "%02d:%02d:%02d", gps.time.hour(), gps.time.minute(), gps.time.second());
        jsonDoc["heure"] = timeBuffer;
      } else {
        jsonDoc["heure"] = nullptr;
      }
      
      // Convertir le document JSON en chaîne JSON
      String jsonString;
      serializeJson(jsonDoc, jsonString);
      
      // Afficher la chaîne JSON
      Serial.println(jsonString);
      
      Serial.println();
    }
  }
}
