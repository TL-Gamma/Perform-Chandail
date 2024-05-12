#include <Arduino.h>
#include <ArduinoJson.h>

// Fonction pour générer un nombre aléatoire dans une plage donnée
float generateRandomValue(float min_value, float max_value) {
  return min_value + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max_value - min_value)));
}

// Fonction pour ajouter un zéro en tête si nécessaire
String addLeadingZero(int value) {
  if (value < 10) {
    return "0" + String(value);
  } else {
    return String(value);
  }
}

// Fonction pour générer une date aléatoire sous forme de chaîne de caractères
String generateRandomDateAsString() {
  int year = random(2020, 2030); // Année (2020-2029)
  int month = random(1, 13); // Mois (1-12)
  int day = random(1, 32); // Jour (1-31)

  return String(year) + "-" + addLeadingZero(month) + "-" + addLeadingZero(day);
}

// Fonction pour générer une heure aléatoire sous forme de chaîne de caractères
String generateRandomTimeAsString() {
  int hour = random(0, 24); // Heures
  int minute = random(0, 60); // Minutes
  int second = random(0, 60); // Secondes

  return addLeadingZero(hour) + ":" + addLeadingZero(minute) + ":" + addLeadingZero(second);
}

void setup() {
  Serial.begin(9600); // Initialisation de la communication série
  srand(time(nullptr)); // Initialisation de la génération de nombres aléatoires
}

void loop() {
  // Paramètres pour la génération des données
  float min_velocity = 0.05;
  float max_velocity = 1.5;
  float min_altitude = 48.0;
  float max_altitude = 49.5;
  float min_latitude = 45.644;
  float max_latitude = 45.645;
  float min_longitude = -73.843;
  float max_longitude = -73.842;

  // Génération des données
  float velocity = generateRandomValue(min_velocity, max_velocity);
  float altitude = generateRandomValue(min_altitude, max_altitude);
  float latitude = generateRandomValue(min_latitude, max_latitude);
  float longitude = generateRandomValue(min_longitude, max_longitude);
  String date = generateRandomDateAsString(); // Générer une date aléatoire
  String time = generateRandomTimeAsString(); // Générer une heure aléatoire

  // Création d'un objet JSON
  StaticJsonDocument<200> jsonDocument;
  jsonDocument["vitesse"] = velocity;
  jsonDocument["altitude"] = altitude;
  jsonDocument["latitude"] = latitude;
  jsonDocument["longitude"] = longitude;
  jsonDocument["date"] = date;
  jsonDocument["heure"] = time;

  // Sérialisation de l'objet JSON
  String jsonString;
  serializeJson(jsonDocument, jsonString);

  // Affichage de l'objet JSON
  Serial.println(jsonString);

  delay(1000); // Attente d'une seconde avant de générer la prochaine donnée
}
