/********************************************************************
Patch Note :


Actuellement une trop grande quantité de donnée est sauvegardé

Il faut réduire la quantité de donnée a la seconde ou a 2 secondes

Egalemement il n'y a pas de led RGB permettant d'indiquer l'état 
du système (Mode bluetooth, récption de donnée...)


********************************************************************/


#include <ArduinoJson.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>

#define GPS_RX_PIN 16
#define GPS_TX_PIN 17
#define BUTTON_PIN 15  // Pin du bouton poussoir


HardwareSerial gpsSerial(2);
TinyGPSPlus gps;
DynamicJsonDocument jsonDoc(512);

File dataFile;  // Déclaration de la variable dataFile

bool isRecording = false;
int fileNumber = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ;
  }
  Serial.println("Moniteur Série : OK");

  Serial.println("Init SD...");
  if (!SD.begin()) {
    Serial.println("Card Mount Failed : Restart Program");
    while(true){
      delay(10);
    }
  }
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

  listDir(SD, "/", 0);
  Serial.println("SD : OK");

  Serial.println("Init GPS...");
  gpsSerial.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  Serial.println("GPS : OK");

  Serial.println("Init Interruption...");
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonPressed, FALLING);
  Serial.println("Interruption : OK...");

  Serial.println("Start");
}

void loop() {
  if (isRecording) {
    startRecording();
  } else {
    Serial.println("BP : OFF");
    delay(1000);  // Attente d'une seconde avant de vérifier à nouveau
  }
}

void buttonPressed() {
  isRecording = !isRecording;  // Inverse l'état d'enregistrement
}

void createNewFile() {
  fileNumber++;
  String filename = "/gps_data_" + String(fileNumber) + ".json";
  Serial.print("File name : ");
  Serial.println(filename);
  dataFile = SD.open(filename, FILE_WRITE);
  if (!dataFile) {
    Serial.println("Erreur lors de la création du fichier !");
 
  }
}

void startRecording() {
  createNewFile();

  while (isRecording) {
    Serial.println("BP : ON");
    while (isRecording && gpsSerial.available() > 0) {

      if (gps.encode(gpsSerial.read())) {
        // Vérification de la validité de chaque donnée
        if (gps.speed.isValid()) {
          jsonDoc["vitesse"] = gps.speed.kmph();
        } else {
          jsonDoc["vitesse"] = "NULL";
        }

        if (gps.altitude.isValid()) {
          jsonDoc["altitude"] = gps.altitude.meters();
        } else {
          jsonDoc["altitude"] = "NULL";
        }

        if (gps.location.isValid()) {
          jsonDoc["latitude"] = gps.location.lat();
          jsonDoc["longitude"] = gps.location.lng();
        } else {
          jsonDoc["latitude"] = "NULL";
          jsonDoc["longitude"] = "NULL";
        }

        if (gps.date.isValid()) {
          char dateBuffer[20];  // Tampon pour stocker la date formatée
          snprintf(dateBuffer, sizeof(dateBuffer), "%04d-%02d-%02d", gps.date.year(), gps.date.month(), gps.date.day());
          jsonDoc["date"] = dateBuffer;
        } else {
          jsonDoc["date"] = "NULL";
        }

        if (gps.time.isValid()) {
          char timeBuffer[20];  // Tampon pour stocker l'heure formatée
          snprintf(timeBuffer, sizeof(timeBuffer), "%02d:%02d:%02d", gps.time.hour(), gps.time.minute(), gps.time.second());
          jsonDoc["heure"] = timeBuffer;
        } else {
          jsonDoc["heure"] = "NULL";
        }

        // Convertir le document JSON en chaîne JSON
        String jsonString;
        serializeJson(jsonDoc, jsonString);

        // Écrire la chaîne JSON dans le fichier
        if (dataFile) {
          dataFile.println(jsonString);
        } else {
          Serial.println("Erreur : Fichier non disponible !");
        }

        Serial.println(jsonString);
        Serial.println();
        //
      }
      //Serial.println("Stop 1");
      
    }
    Serial.println("Serial disconection, wait restart");
    
  }
  Serial.println("Stop 3");
  stopRecording();
}

void stopRecording() {
  Serial.println("Enregistrement des données GPS arrêté.");
  if (dataFile) {
    dataFile.close();
  }
}

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
        listDir(fs, file.path(), levels - 1);
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