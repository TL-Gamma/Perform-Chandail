//Bibliothèques
#include <ArduinoJson.h>        // Bibliothèque pour manipuler les objets JSON
#include <TinyGPS++.h>          // Bibliothèque pour interagir avec le module GPS
#include <HardwareSerial.h>     // Bibliothèque pour utiliser la liaison série matérielle
#include <FS.h>                 // Bibliothèque pour manipuler le système de fichiers
#include <SD.h>                 // Bibliothèque pour interagir avec la carte SD
#include <SPI.h>                // Bibliothèque pour la communication SPI
#include <LiquidCrystal_I2C.h>  // Bibliothèque pour interagir avec l'écran LCD I2C

//Constantes de pins
#define GPS_RX_PIN 16      // Broche de réception du signal GPS (RX2)
#define GPS_TX_PIN 17      // Broche de transmission du signal GPS (TX2)
#define INTERUPTOR_PIN 34  // Broche du bouton poussoir
#define PINLED_R 25        // Broche de la LED RGB rouge
#define PINLED_G 27        // Broche de la LED RGB verte
#define PINLED_B 26        // Broche de la LED RGB bleue

// Constantes de couleurs
// R , V , B
#define WHITE 128, 255, 255
#define RED 255, 0, 0
#define GREEN 0, 255, 0
#define BLUE 0, 0, 255
#define CYAN 0, 255, 255
#define MAGENTA 128, 0, 255
#define YELLOW 128, 255, 0

//Constantes de calcul
#define INTERVALLE_ACQUISITION 1000  // Intervalle entre chaque acquisition de données GPS (en millisecondes)
#define BAUD_RATE 9600               // Vitesse de communication du moniteur série
#define VOLTAGE_DIVIDER_FACTOR 2.3   // Facteur de multiplication de la tension pour retrouver la tension avant le pont diviseur
#define BATTERY_LOW_THRESHOLD 3.6    // Limite minimale de tension pour préserver la batterie


// Initialisation des bibliothèques
HardwareSerial gpsSerial(2);  // Initialisation de la liaison série matérielle pour le GPS
TinyGPSPlus gps;              // Création d'une instance de l'objet TinyGPSPlus pour interagir avec le GPS

DynamicJsonDocument jsonDoc(512);  // Création d'un document JSON dynamique de taille 512 octets
File dataFile;                     // Déclaration de la variable dataFile pour gérer les fichiers sur la carte SD

LiquidCrystal_I2C lcd(0x27, 20, 4);  // Configuration de l'adresse de l'écran à 0x27 pour un affichage 16 caractères sur 2 lignes

// Variables globales
bool isRecording = false;  // Variable booléenne pour indiquer si l'enregistrement est en cours
int fileNumber = 0;        // Numéro du fichier de données
String filename = "";
char lastFilename[MAX_FILENAME_LENGTH];  // Variable pour stocker le nom du dernier fichier créé

// Prototypes :
bool mode();
float readBattLevel();
void displayBat();
void createNewFile(const char *chemin);
int trouver_prochain_numero(const char *chemin);
void readGPSData();
void checkGPSValidity();
void recording();
void setColor(uint8_t redValue, uint8_t greenValue, uint8_t blueValue);
void ledBlink(uint8_t redValue, uint8_t greenValue, uint8_t blueValue);
void ledOff();
void listDir(fs::FS &fs, const char *dirname, uint8_t levels);
void cardType();
void EEPROM_init();
void EEPROM_findLastFile();
void EEPROM_writeString(int addr, const String &str);
String EEPROM_readString(int addr);
void EEPROM_reset();



///////////////////////////////////////////////////////// SETUP ////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(BAUD_RATE);  // Initialisation de la communication série
  Serial.println("Moniteur Série : OK");

  // Configuration du LCD
  Serial.println("Init LCD...");
  LCD_config();
  Serial.println("LCD : OK");

  // Initialisation des broches pour les LED RGB
  Serial.println("Init LED RGB...");
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
      ledBlink(RED);
    }
  }
  cardType();
  listDir(SD, "/", 0);  // Affichage du contenu du répertoire racine de la carte SD
  // Implémenter une fonctionnalité permettant d'empecher l'écriture d'un nouveau fichier si l'espace sur la SD nécessaire pour une future course n'est pas possible (de l'ordre de 100Ko)
  Serial.println("SD : OK");

  // Initialisation du module GPS
  Serial.println("Init GPS...");
  gpsSerial.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  Serial.println("GPS : OK");

  // Configuration de l'interrupeur au démarrage
  Serial.println("Init Interrupteur...");
  pinMode(INTERUPTOR_PIN, INPUT);
  Serial.println("Interrupteur : OK");

  
  // Initialisation des 2 modes
  if (mode() == true) {  // Initialiation du mode course
    Serial.println("Init mode Course:");
    createNewFile("");
    //Patch d'affichage moche
    lcd.setCursor(0, 1);
    lcd.print("Lat:                ");
  } else {
    Serial.println("Init mode BT:");
    
  }
 


  // Animation LED au démarrage
  Serial.println("Start");
  for (int i = 0; i < 3; i++) {
    ledBlink(WHITE);
  }
}

///////////////////////////////////////////////////////// LOOP ////////////////////////////////////////////////////////////
void loop() {
  displayBat();
  if (mode() == true) {
    recording();
  } else {
    ledBlink(BLUE);
    return;
  }
  delay(500);  // Attente de 500 millisecondes avant la prochaine lecture de la tension de la batterie
}

///////////////////////////////////////////////////////// FONCTIONS ////////////////////////////////////////////////////////////

// Lecture du niveau de batterie
float readBattLevel() {
  int sensorValue = analogRead(INTERUPTOR_PIN);                           // Lecture de la tension analogique
  float voltage = (sensorValue * (3.3 / 4095) * VOLTAGE_DIVIDER_FACTOR);  // Conversion de la valeur analogique en tension
  return voltage;
}


void displayBat() {
  // Lecture de la tension de la batterie
  float voltage = readBattLevel();
  // Affichage de la tension de la batterie dans la console série
  Serial.print("Tension : ");
  Serial.println(voltage);
  // Affichage de la tension de la batterie sur l'écran LCD

  LCD_displayVoltage(voltage);
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

void ledBlink(uint8_t redValue, uint8_t greenValue, uint8_t blueValue) {
  setColor(redValue, greenValue, blueValue);
  delay(250);
  ledOff();
  delay(250);
}


// Fonction pour éteindre les LED RGB
void ledOff() {
  analogWrite(PINLED_R, 255);
  analogWrite(PINLED_G, 255);
  analogWrite(PINLED_B, 255);
}

bool mode() {
  float voltage = readBattLevel();  // Lecture de la tension de la batterie
  if (voltage > 0) {                // Si la batterie a de la tension
    Serial.println("Mode course");  // Affichage du mode dans la console série
    LCD_displayMode("Course");
    return true;
  } else {
    Serial.println("Mode BT");  // Affichage du mode dans la console série
    LCD_displayMode("BT");
    return false;
  }
}

// Fonction pour créer un nouveau fichier de données GPS

int trouver_prochain_numero(const char *chemin) {
  int numero = 1;
  char nom_fichier[50];
  // Tant que le fichier existe, on incrémente le numéro
  do {
    snprintf(nom_fichier, sizeof(nom_fichier), "%s/gps_data_%d.json", chemin, numero);
    if (!SD.exists(nom_fichier)) {
      break;
    }
    numero++;
  } while (true);
  return numero;
}

void createNewFile(const char *chemin) {
  if (!mode()) {  // Si le mode n'est pas "course", quitter la fonction
    return;
  }

  int fileNumber = trouver_prochain_numero(chemin);
  filename = String(chemin) + "/gps_data_" + String(fileNumber) + ".json";  // Nom du fichier
  Serial.print("Nom du fichier : ");
  Serial.println(filename);

  dataFile = SD.open(filename, FILE_WRITE);  // Ouverture du fichier en mode écriture
  if (!dataFile) {
    Serial.println("Erreur lors de l'ouverture du fichier a la création!");  // Erreur lors de l'ouverture du fichier
    ledBlink(MAGENTA);
    return;
  }

  // Écriture de la première partie du fichier JSON
  if (dataFile.println("[") == 0) {
    Serial.println("Erreur lors de l'écriture de '[' dans le fichier!");
    ledBlink(MAGENTA);
    dataFile.close();
    return;
  }

  Serial.println("Fichier créé avec succès!");
  ledBlink(YELLOW);
  dataFile.close();  // Fermeture du fichier après l'initialisation
}




void readGPSData() {
  if (gpsSerial.available() > 0) {
    while (gpsSerial.available() > 0) {
      gps.encode(gpsSerial.read());
    }
  }
}

void checkGPSValidity() {
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
    LCD_displayLat(gps.location.lat());
    LCD_displayLng(gps.location.lng());
  } else {
    jsonDoc["latitude"] = nullptr;
    jsonDoc["longitude"] = nullptr;
    LCD_displayLat(-99.99999999);
    LCD_displayLng(-99.99999999);
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
    LCD_displayHour(timeBuffer);
  } else {
    jsonDoc["heure"] = nullptr;
    LCD_displayHour("\0");
  }
}


void recording() {
  static unsigned long dernierTempsAcquisition = 0;
  static bool isFirstEntry = true;
  if (millis() - dernierTempsAcquisition >= INTERVALLE_ACQUISITION) {
    dernierTempsAcquisition = millis();
    readGPSData();
    checkGPSValidity();
    String currentJsonString;
    serializeJson(jsonDoc, currentJsonString);

    Serial.println("Enregistrement des données...");

    // Ouvrir le fichier en mode ajout
    File dataFile = SD.open(filename, FILE_APPEND);
    if (dataFile) {
      Serial.print("Fichier ouvert :");
      Serial.println(filename);

      // Vérifier si c'est la première fois que l'on entre dans la boucle
      if (!isFirstEntry) {
        // Si ce n'est pas la première fois, vérifier le dernier caractère
        dataFile.seek(dataFile.size() - 1);
        char lastChar = dataFile.read();
        dataFile.seek(dataFile.size());

        // Ajouter la virgule si le dernier caractère est une "]""
        if (lastChar == ']') {
          dataFile.print(",");
        }
      } else {
        // Si c'est la première fois, mettre isFirstEntry à false
        isFirstEntry = false;
      }

      // Ajouter la nouvelle entrée JSON
      dataFile.println(currentJsonString);

      // Fermer le fichier
      dataFile.close();
      Serial.println("Fichier fermé");
      ledBlink(GREEN);
    } else {
      Serial.println("Erreur lors de l'ouverture du fichier pendant l'écriture!");  // Erreur lors de l'ouverture du fichier
      ledBlink(MAGENTA);
    }
  }
}


void LCD_config() {
  lcd.init();       // Initialisation de l'écran LCD
  lcd.clear();      // Effacement de l'écran LCD
  lcd.backlight();  // Activation du rétroéclairage de l'écran LCD
  lcd.setCursor(0, 0);
  lcd.print("Mode:");
  lcd.setCursor(12, 0);
  lcd.print("| U:");
  lcd.setCursor(0, 1);
  lcd.print("Lat:");
  lcd.setCursor(0, 2);
  lcd.print("Lng:");
  lcd.setCursor(0, 3);
  lcd.print("Hour:");
}

void LCD_displayVoltage(float value) {
  lcd.setCursor(16, 0);  // Positionnement du curseur à la deuxième ligne, onzième caractère
  lcd.print(value, 2);   // Affichage de la tension
}
void LCD_displayMode(String value) {
  lcd.setCursor(5, 0);  // Positionnement du curseur à la deuxième ligne, onzième caractère
  if (value == "\0") {
    lcd.print("null");
  } else {
    lcd.print(value);  // Affichage de la tension
  }
}

void LCD_displayLat(double value) {
  lcd.setCursor(5, 1);  // Positionnement du curseur à la deuxième ligne, onzième caractère
  lcd.print(value, 8);  // Affichage de la tension
}

void LCD_displayLng(double value) {
  lcd.setCursor(5, 2);  // Positionnement du curseur à la deuxième ligne, onzième caractère
  lcd.print(value, 8);  // Affichage de la tension
}

void LCD_displayHour(String value) {
  lcd.setCursor(6, 3);  // Positionnement du curseur à la deuxième ligne, onzième caractère
  if (value == "\0") {
    lcd.print("null");
  } else {
    lcd.print(value);  // Affichage de la tension
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
void cardType() {
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
}
