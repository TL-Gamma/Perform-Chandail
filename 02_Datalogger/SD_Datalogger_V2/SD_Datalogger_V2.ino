#include <ArduinoJson.h>      // Bibliothèque pour manipuler les objets JSON
#include <TinyGPS++.h>        // Bibliothèque pour interagir avec le module GPS
#include <HardwareSerial.h>   // Bibliothèque pour utiliser la liaison série matérielle
#include <FS.h>               // Bibliothèque pour manipuler le système de fichiers
#include <SD.h>               // Bibliothèque pour interagir avec la carte SD
#include <SPI.h>              // Bibliothèque pour la communication SPI

#define GPS_RX_PIN 16         // Broche de réception du signal GPS (RX)
#define GPS_TX_PIN 17         // Broche de transmission du signal GPS (TX)
#define BUTTON_PIN 15         // Broche du bouton poussoir

#define PINLED_R 25           // Broche de la LED RGB rouge
#define PINLED_B 26           // Broche de la LED RGB bleue
#define PINLED_G 27           // Broche de la LED RGB verte
#define INTERVALLE_ACQUISITION 1000  // Intervalle entre chaque acquisition de données GPS (en millisecondes)

HardwareSerial gpsSerial(2);  // Initialisation de la liaison série matérielle pour le GPS
TinyGPSPlus gps;              // Création d'une instance de l'objet TinyGPSPlus pour interagir avec le GPS
DynamicJsonDocument jsonDoc(512);  // Création d'un document JSON dynamique de taille 512 octets
File dataFile;                 // Déclaration de la variable dataFile pour gérer les fichiers sur la carte SD
bool isRecording = false;      // Variable booléenne pour indiquer si l'enregistrement est en cours
int fileNumber = 0;            // Numéro du fichier de données

// Fonction de configuration, exécutée une seule fois au démarrage
void setup() {
  Serial.begin(9600);         // Initialisation de la communication série
  while (!Serial) {
    ;
  }
  Serial.println("Moniteur Série : OK");

  // Initialisation des broches pour les LED RGB
  pinMode(PINLED_R, OUTPUT);
  pinMode(PINLED_G, OUTPUT);
  pinMode(PINLED_B, OUTPUT);
  Serial.println("LED RGB : OK");
  ledOff();  // Éteindre les LED au démarrage

  // Initialisation de la carte SD
  Serial.println("Init SD...");
  if (!SD.begin()) {  // Vérification de la connexion de la carte SD
    while (true) {
      Serial.println("Card Mount Failed : Connect SD & Restart ESP32");
      setColor(255, 0, 0);  // LED rouge pour signaler une erreur
      delay(100);
    }
  }

  // Vérification du type de carte SD
  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }
  Serial.print("SD Card Type: ");
  if (cardType == CARD_MMC) {
    Serial.println("MMC");
  } else if (cardType == CARD_SD) {
    Serial.println("SDSC");
  } else if (cardType == CARD_SDHC) {
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }

  listDir(SD, "/", 0);  // Affichage du contenu du répertoire racine de la carte SD
  Serial.println("SD : OK");

  // Initialisation du module GPS
  Serial.println("Init GPS...");
  gpsSerial.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  Serial.println("GPS : OK");

  // Configuration de l'interruption pour le bouton poussoir
  Serial.println("Init Interruption...");
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonPressed, FALLING);  // Interruption sur front descendant
  Serial.println("Interruption : OK");

  // Animation LED au démarrage
  Serial.println("Start");
  for (int i = 0; i < 3; i++) {
    setColor(255, 255, 255);  // LED blanche
    delay(100);
    ledOff();  // Éteindre les LED
    delay(100);
  }
}

// Boucle principale du programme, exécutée en continu après la configuration
void loop() {
  if (isRecording) {
    Serial.println("BP : ON");
    createNewFile();     // Création d'un nouveau fichier pour enregistrer les données GPS
    startRecording();    // Démarrage de l'enregistrement des données
  } else {
    Serial.println("BP : OFF");
    Serial.println("No SD card attached");  // Aucune carte SD connectée
    stopRecording();     // Arrêt de l'enregistrement
    setColor(0, 0, 255); // LED bleue pour indiquer l'arrêt de l'enregistrement
    delay(1000);
    ledOff();
    delay(1000);
  }
}

// Fonction pour définir la couleur des LED RGB
void setColor(uint8_t redValue, uint8_t greenValue, uint8_t blueValue) {
  redValue = 255 - redValue;
  greenValue = 255 - greenValue;
  blueValue = 255 - blueValue;
  analogWrite(PINLED_R, redValue);
  analogWrite(PINLED_G, greenValue);
  analogWrite(PINLED_B, blueValue);
}

// Fonction pour éteindre les LED RGB
void ledOff() {
  analogWrite(PINLED_R, 255);
  analogWrite(PINLED_G, 255);
  analogWrite(PINLED_B, 255);
}

// Fonction appelée lors de l'appui sur le bouton poussoir
void buttonPressed() {
  isRecording = !isRecording;  // Inversion de l'état d'enregistrement
}

// Fonction pour créer un nouveau fichier de données GPS
void createNewFile() {
  fileNumber++;
  String filename = "/gps_data_" + String(fileNumber) + ".json";  // Nom du fichier
  Serial.print("File name : ");
  Serial.println(filename);
  dataFile = SD.open(filename, FILE_WRITE);  // Ouverture du fichier en mode écriture
  if (!dataFile) {
    isRecording = false;
    Serial.println("Erreur lors de la création du fichier !");  // Erreur lors de l'ouverture du fichier
  } else
    dataFile.print("[\n");  // Écriture du premier crochet ouvrant dans le fichier JSON
}

// Fonction pour démarrer l'enregistrement des données GPS
void startRecording() {
  unsigned long dernierTempsAcquisition = 0;  // Variable pour stocker le temps de la dernière acquisition
  bool isFirstEntry = true;                   // Pour gérer la première entrée
  String lastJsonString = "";                 // Stocke la dernière chaîne JSON enregistrée

  // Boucle d'enregistrement des données GPS
  while (isRecording) {
    // Vérifie si le temps écoulé depuis la dernière acquisition est supérieur ou égal à l'intervalle défini
    if (millis() - dernierTempsAcquisition >= INTERVALLE_ACQUISITION) {
      dernierTempsAcquisition = millis();  // Met à jour le temps de la dernière acquisition
      if (gpsSerial.available() > 0) {
        // Lecture des données GPS
        while (gpsSerial.available() > 0) {
          gps.encode(gpsSerial.read());
        }
        // Vérification de la validité des données GPS
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
          char dateBuffer[20];  // Tampon pour stocker la date formatée
          snprintf(dateBuffer, sizeof(dateBuffer), "%04d-%02d-%02d", gps.date.year(), gps.date.month(), gps.date.day());
          jsonDoc["date"] = dateBuffer;
        } else {
          jsonDoc["date"] = nullptr;
        }
        if (gps.time.isValid()) {
          char timeBuffer[20];  // Tampon pour stocker l'heure formatée
          snprintf(timeBuffer, sizeof(timeBuffer), "%02d:%02d:%02d", gps.time.hour(), gps.time.minute(), gps.time.second());
          jsonDoc["heure"] = timeBuffer;
        } else {
          jsonDoc["heure"] = nullptr;
        }
        // Conversion de l'objet JSON en chaîne JSON
        String currentJsonString;
        serializeJson(jsonDoc, currentJsonString);
        if (currentJsonString != lastJsonString) {
          // Enregistrement de la trame JSON sur la carte SD
          if (dataFile) {
            // Ajout d'une virgule avant chaque élément sauf le premier
            if (!isFirstEntry) {
              dataFile.print(",");
              Serial.print(",");
            } else {
              isFirstEntry = false;
            }
            dataFile.println(currentJsonString);
            Serial.println(currentJsonString);

            setColor(0, 255, 0);  // LED verte pour indiquer l'enregistrement des données
            delay(100);
            ledOff();  // Éteindre les LED
            delay(100);
            // Mise à jour de la dernière chaîne JSON enregistrée
            lastJsonString = currentJsonString;
          } else {
            Serial.println("Erreur : Fichier non disponible !");
            setColor(255, 255, 0);  // LED jaune pour indiquer une erreur
            delay(100);
            ledOff();  // Éteindre les LED
            delay(100);
          }
        }

      } else {
        setColor(255, 0, 0);  // LED rouge pour indiquer des données GPS incomplètes
        delay(100);
        ledOff();  // Éteindre les LED
        delay(100);
        Serial.println("Données GPS Incompletes, trame non enregistrée.");
      }
    }
    delay(10);
  }
}

// Fonction pour arrêter l'enregistrement des données GPS
void stopRecording() {
  Serial.println("Enregistrement des données GPS arrêté.");
  if (dataFile) {
    dataFile.print("]\n");  // Écriture du crochet fermant dans le fichier JSON
    dataFile.close();       // Fermeture du fichier
  }
}

// Fonction pour lister les fichiers dans un répertoire
void listDir(fs::FS &fs, const char *dirname, uint8_t levels) {
  Serial.printf("Listing directory: %s\n", dirname);
  File root = fs.open(dirname);
  if (!root) {
    Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("Not a directory");
    return;
  }
  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels) {
        listDir(fs, file.path(), levels - 1);  // Récursivité pour les sous-répertoires
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}
