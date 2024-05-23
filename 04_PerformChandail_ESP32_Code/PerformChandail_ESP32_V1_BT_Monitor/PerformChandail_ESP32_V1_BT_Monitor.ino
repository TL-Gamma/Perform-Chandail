//Bibliothèques
#include <ArduinoJson.h>        // Bibliothèque pour manipuler les objets JSON
#include <TinyGPS++.h>          // Bibliothèque pour interagir avec le module GPS
#include <HardwareSerial.h>     // Bibliothèque pour utiliser la liaison série matérielle
#include <FS.h>                 // Bibliothèque pour manipuler le système de fichiers
#include <SD.h>                 // Bibliothèque pour interagir avec la carte SD
#include <SPI.h>                // Bibliothèque pour la communication SPI
#include <WiFi.h>               // Bibliothèque pour  le Wifi
#include <AsyncTCP.h>           // Bibliothèque TCP
#include <ESPAsyncWebServer.h>  // Blibliothèque Server HTTP

#include <BluetoothSerial.h> // Bibliothèque pour le monitorage Bluetooth Serial
// Constante de mode
#define GPS_DATALOGGER false
#define SD_JSON_STACK false
//Constantes de pins
#define GPS_RX_PIN 16      // Broche de réception du signal GPS (RX2)
#define GPS_TX_PIN 17      // Broche de transmission du signal GPS (TX2)
#define INTERUPTOR_PIN 34  // Broche de l'interrupteur ON/OFF
#define PINLED_R 27        // Broche de la LED RGB rouge
#define PINLED_B 26        // Broche de la LED RGB bleue
#define PINLED_G 25        // Broche de la LED RGB verte
#define PUSHBUTTON_PIN 15  // Broche du bouton de fin de course

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
#define VOLTAGE_DIVIDER_FACTOR 2.2   // Facteur de multiplication de la tension pour retrouver la tension avant le pont diviseur
#define BATTERY_LOW_THRESHOLD 3.6    // Limite minimale de tension pour préserver la batterie
#define TEMPS_PRESSION 1000          // Temps de consideration d'une pression longe pour la fin de l'aquisiton des données (en milisecondes)
#define DEFAULT_FILE "/gps_data.json"

const char *ssid = "PerformChandail";
const char *password = "12345678";

AsyncWebServer server(80);

// Initialisation des bibliothèques
HardwareSerial gpsSerial(2);  // Initialisation de la liaison série matérielle pour le GPS
TinyGPSPlus gps;              // Création d'une instance de l'objet TinyGPSPlus pour interagir avec le GPS
// Création des documents JSON Dynamiques
DynamicJsonDocument gpsJsonDoc(512);   // Création d'un document JSON dynamique de taille 512 octets
DynamicJsonDocument dataJsonDoc(512);  // Création d'un document JSON dynamique de taille 512 octets
File dataFile;                         // Déclaration de la l'instance dataFile pour gérer les fichiers sur la carte SD  coté datalogger

// Variables globales
bool endRecord = false;  // Variable booléenne pour indiquer si l'enregistrement à été aretté
int fileNumber = 0;      // Numéro du fichier de données
String filename = "";
unsigned long tempsDebutPression = 0;  // Variable pour enregistrer le temps au début de la pression


// Prototypes
bool mode();
float readBattLevel();
void displayBat();
void createNewFile(const char *chemin);
int trouver_prochain_numero(const char *chemin);
void readGPSData();
void getGPSData();
void recording();
void stopRecording();
void buttonPressed();
void setColor(uint8_t redValue, uint8_t greenValue, uint8_t blueValue);
void ledBlink(uint8_t redValue, uint8_t greenValue, uint8_t blueValue);
void ledOff();
void listDir(fs::FS &fs, const char *dirname, uint8_t levels);
void cardType();


////////////////////////////////////////////// CLASSE DE TRAITEMENT DES DONNEES ///////////////////////////////////////////////

class GPSData {
public:
  ///////////////////////////////////// CALCULDE DU DENIVLE POSITIF /////////////////////////////////////////////////

  void calculatePositiveAltitudeChange(float altitudeGPS) {
    // Vérifier si l'altitude GPS est valide
    if (altitudeGPS > previousAltitude) {
      // Calculer la différence d'altitude positive et l'ajouter au dénivelé positif total
      positiveAltitudeChange += altitudeGPS - previousAltitude;
    }
    // Mettre à jour l'altitude précédente pour le prochain appel de fonction
    previousAltitude = altitudeGPS;
  }

  float getPositiveAltitudeChange() const {
    return positiveAltitudeChange;
  }


  ///////////////////////////////////// CALCUL DE LA DISTENCE TOTALE DE COURSE //////////////////////////////////////

  void calculateTotalDistance(float lat, float lon) {
    // Calculer la distance entre la position actuelle et la position précédente
    if (prevLat != 0 && prevLon != 0) {
      totalDistance += TinyGPSPlus::distanceBetween(prevLat, prevLon, lat, lon);
    }
    // Mettre à jour la position précédente
    prevLat = lat;
    prevLon = lon;
  }

  float getTotalDistance() const {
    return totalDistance;
  }

  ///////////////////////////////////// CALCUL DU TEMPS DE COURSE //////////////////////////////////////

  void startTimer() {
    startTime = millis();  // Record the start time
  }
  String getDuration() {
    unsigned long currentTime = millis();                            // Get the current time
    unsigned long totalTime = currentTime - startTime;               // Calculate the elapsed time since the start of the race
    unsigned long hours = totalTime / 3600000;                       // Calculate the number of hours
    unsigned long minutes = (totalTime % 3600000) / 60000;           // Calculate the number of minutes
    unsigned long seconds = ((totalTime % 3600000) % 60000) / 1000;  // Calculate the number of seconds

    // Format the total time in HH:MM:SS format
    char buffer[9];  // "HH:MM:SS\0"
    snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", hours, minutes, seconds);

    return String(buffer);
  }

  ///////////////////////////////////// CALCUL DE LA VITESSE MOYENNE DE LA COURSE //////////////////////////////////////

  void calculateAverageSpeed(float gpsSpeed) {
    // Add the GPS speed to the sum of speeds
    sumSpeeds += gpsSpeed;
    // Increment the number of measurements
    numberOfMeasurements++;
  }

  float getAverageSpeed() const {
    if (numberOfMeasurements > 0) {
      return sumSpeeds / numberOfMeasurements;
    } else {
      return 0.0;  // Return 0 if no measurements have been made yet
    }
  }

private:
  // Variables durée
  unsigned long startTime = 0;

  // Variables vitesse moyenne
  float sumSpeeds = 0.0;
  int numberOfMeasurements = 0;

  // Variables distence
  double prevLat = 0.0;        // Latitude précédente
  double prevLon = 0.0;        // Longitude précédente
  double totalDistance = 0.0;  // Distance totale parcourue

  // Variables de dénivlé positif
  float positiveAltitudeChange = 0.0;  // Dénivelé positif total
  float previousAltitude = 0.0;        // Altitude précédente
};

// Creating an instance of the GPSData class
GPSData gpsData;


///////////////////////////////////////////////////////// SETUP ////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(BAUD_RATE);  // Initialisation de la communication série
  Serial.println("Moniteur Série : OK");

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
  // Implémenter une fonctionnalité permettant d'empecher l'écriture d'un nouveau fichier si l'espace sur la SD nécessaire pour une future course n'est pas possible (de l'ordre de 200Ko)
  Serial.println("SD : OK");



  // Configuration de l'interrupeur au démarrage
  Serial.println("Init Interrupteur...");
  pinMode(INTERUPTOR_PIN, INPUT);
  Serial.println("Interrupteur : OK");

  // Configuration du bouton poussoir d'arret de course
  Serial.println("Init Push button...");
  pinMode(PUSHBUTTON_PIN, INPUT_PULLUP);
  Serial.println("Push button : OK");

  // Initialisation des 2 modes
  if (mode() == true) {  // Initialiation du mode course
    Serial.println("Init mode Course:");
    // Initialisation du module GPS
    Serial.println("Init GPS...");
    gpsSerial.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
    Serial.println("GPS : OK");

    createNewFile("");


  } else {
    // Create Wifi AP
    Serial.println("Init mode Server HTTP:");
    WiFi.softAP(ssid, password);
    Serial.println("WiFi connected");
    Serial.print("Wifi Name :");
    Serial.println(ssid);
    Serial.print("PWD :");
    Serial.println(password);
    // Print the IP address of the server
    Serial.print("Server IP address: ");
    Serial.println(WiFi.softAPIP());

    // Define API endpoints
    server.on("/", HTTP_GET, handleRoot);
    // ROOT SERVER

    /////// UNIQUEMENT EN MODE SD_JSON_STACK true ///////
    server.on("/list_files", HTTP_GET, handleListFiles);
    //EXEMPLE DE REQUETTE LISTFILE : http://192.168.4.1/list_files

    server.on("/download", HTTP_GET, handleDownload);  // Télécharge le fichier
    //EXEMPLE DE REQUETTE DOWNLOAD : http://192.168.4.1/download?file=/gps_data_1.json

    server.on("/remove_file", HTTP_GET, handleRemoveFile);  // Supprime un fichié pointé sur la carte SD
    //EXEMPLE DE REQUETTE REMOVEFILE : http://192.168.4.1/remove_file?file=/gps_data_1.json

    server.on("/pointed_gps_data", HTTP_GET, handlePointedGpsData);  // Pousse le fichier JSON pointé sur l'appli mobile
    //EXEMPLE DE REQUETTE DOWNLOAD : http://192.168.4.1/pointed_gps_data?file=/gps_data_1.json

    //////// UNIQUEMENT EN MODE SD_JSON_STACK false ///////

    server.on("/gps_data", HTTP_GET, handleGpsData);  // Pousse le JSON courant sur l'appli mobile
    //EXEMPLE DE REQUETTE /gps_data : http://192.168.4.1/gps_data

    server.on("/get_json_data", HTTP_GET, handleGpsValue);  // Pousse le JSON courant sur l'appli mobile
    //EXEMPLE DE REQUETTE /get_json_data : http://192.168.4.1/get_json_data?value=date


    // Start server
    server.begin();
    Serial.println("HTTP server started");
  }

  // Animation LED au démarrage
  Serial.println("Start");
  for (int i = 0; i < 3; i++) {
    ledBlink(WHITE);
  }
  gpsData.startTimer();
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
  delay(500);  // Attente de 500 millisecondes pour la lecture du moniteur série
}

///////////////////////////////////////////////////////// FONCTIONS ////////////////////////////////////////////////////////////

// Fonction appelée lors de l'appui sur le bouton poussoir
void buttonPressed() {
  endRecord = !endRecord;  // Inversion de l'état d'enregistrement
}

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
  if (voltage < BATTERY_LOW_THRESHOLD && mode() == true) {  // Si la tension de la batterie pendant la course est inférieure au seuil de préservation de la batterie
    stopRecording();
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
    return true;
  } else {
    Serial.println("Mode BT");  // Affichage du mode dans la console série
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

#if SD_JSON_STACK == true
  int fileNumber = trouver_prochain_numero(chemin);
  filename = String(chemin) + "/gps_data_" + String(fileNumber) + ".json";  // Nom du fichier
  Serial.print("Nom du fichier : ");
  Serial.println(filename);
#else
  if (SD.exists("/gps_data.json")) {
    // Suppression du fichier s'il existe
    if (SD.remove("/gps_data.json")) {
      Serial.println("Fichier gps_data.json existant supprimé avec succès");
      filename = "/gps_data.json";
    } else {
      Serial.println("Erreur lors de la suppression du fichier gps_data.json");
    }
  } else {
    filename = "/gps_data.json";
  }
#endif


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

void getGPSData() {
  if (gps.speed.isValid()) {
    gpsData.calculatePositiveAltitudeChange(gps.speed.kmph());
#if GPS_DATALOGGER == true
    gpsJsonDoc["vitesse"] = gps.speed.kmph();
#endif
  } else {
#if GPS_DATALOGGER == true
    gpsJsonDoc["vitesse"] = nullptr;
#endif
  }
  if (gps.altitude.isValid()) {
    gpsData.calculatePositiveAltitudeChange(gps.altitude.meters());
#if GPS_DATALOGGER == true
    gpsJsonDoc["altitude"] = gps.altitude.meters();
#endif
  } else {
#if GPS_DATALOGGER == true
    gpsJsonDoc["altitude"] = nullptr;
#endif
  }
  if (gps.location.isValid()) {
    gpsData.calculateTotalDistance(gps.location.lat(), gps.location.lng());
#if GPS_DATALOGGER == true
    gpsJsonDoc["latitude"] = gps.location.lat();
    gpsJsonDoc["longitude"] = gps.location.lng();
#endif
  } else {
#if GPS_DATALOGGER == true
    gpsJsonDoc["latitude"] = nullptr;
    gpsJsonDoc["longitude"] = nullptr;
#endif
  }
#if GPS_DATALOGGER == true
  if (gps.time.isValid()) {
    char timeBuffer[20];  // Tampon pour stocker l'heure formatée
    snprintf(timeBuffer, sizeof(timeBuffer), "%02d:%02d:%02d", gps.time.hour(), gps.time.minute(), gps.time.second());
    gpsJsonDoc["time"] = timeBuffer;
  } else {
    gpsJsonDoc["time"] = nullptr;
  }
#endif
}


String getDate() {
  readGPSData();
  if (gps.date.isValid()) {
    char dateBuffer[20];  // Tampon pour stocker la date formatée
    snprintf(dateBuffer, sizeof(dateBuffer), "%04d-%02d-%02d", gps.date.year(), gps.date.month(), gps.date.day());
    return String(dateBuffer);  // Ajout du point-virgule à la fin de la ligne de retour
  }
}
void recording() {
  static unsigned long dernierTempsAcquisition = 0;
  static bool isFirstEntry = true;


  if (millis() - dernierTempsAcquisition >= INTERVALLE_ACQUISITION) {
    dernierTempsAcquisition = millis();
    readGPSData();
    getGPSData();

    String currentJsonString;
    serializeJson(gpsJsonDoc, currentJsonString);

    Serial.println("Enregistrement des données...");

    // Ouvrir le fichier en mode ajout
    File dataFile = SD.open(filename, FILE_APPEND);

    if (dataFile) {
      Serial.print("Fichier ouvert :");
      Serial.println(filename);
#if GPS_DATALOGGER == true
      // Ajouter la nouvelle entrée JSON
      dataFile.print(currentJsonString);
      dataFile.println(",");
#endif
      Serial.print("JSON : ");
      Serial.println(currentJsonString);
      // Fermer le fichier
      dataFile.close();
      Serial.println("Fichier fermé");
      ledBlink(GREEN);

      // Détection de la pression longue
      if (digitalRead(PUSHBUTTON_PIN) == LOW) {
        if (tempsDebutPression == 0) {
          tempsDebutPression = millis();
        }
        if (millis() - tempsDebutPression >= TEMPS_PRESSION) {
          Serial.println("Fin de course");
          ledBlink(CYAN);
          stopRecording();
          tempsDebutPression = 0;  // Réinitialiser le temps de début de pression pour éviter la répétition des actions
        }
      }
    } else {
      Serial.println("Erreur lors de l'ouverture du fichier pendant l'écriture!");  // Erreur lors de l'ouverture du fichier
      ledBlink(MAGENTA);
    }
  }
}


// Fonction pour arrêter l'enregistrement et enregistrer les données finales
void stopRecording() {
  File dataFile = SD.open(filename, FILE_APPEND);
  if (dataFile) {
    // Écrire les données dans le fichier JSON avec des sauts de ligne
    dataFile.println("\t{");
    dataFile.println("\t\"date\": \"" + String(getDate()) + "\",");
    dataFile.println("\t\"duration\": \"" + String(gpsData.getDuration()) + "\",");
    dataFile.println("\t\"average_speed\": \"" + String(gpsData.getAverageSpeed()) + "\",");
    dataFile.println("\t\"denivle_positif\": \"" + String(gpsData.getPositiveAltitudeChange()) + "\",");
    dataFile.println("\t\"total_distance\": \"" + String(gpsData.getTotalDistance()) + "\"");
    dataFile.println("\t}");
    dataFile.println("]");
    dataFile.close();
    ledBlink(GREEN);
    esp_deep_sleep_start();  // Mise en veille profonde de l'ESP32
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

/////////////////////////////////////////////////////////////////////////////////////////// HTTP SERVER REQUEST /////////////////////////////////////////////////////////////////////////////////////////////

void handleRoot(AsyncWebServerRequest *request) {
  String response = "Bienvenue ! Voici les commandes disponibles :<br>";
  response += "<a href=\"/list_files\">Lister les fichiers</a><br>";

  response += "<a href=\"/download?file=/gps_data_1.json\">Télécharger le fichier pointé vers l'application mobile</a><br>";
  response += "<a href=\"/remove_file?file=/gps_data_1.json\">Supprimer un fichier JSON</a><br>";
  response += "<a href=\"/pointed_gps_data?file=/gps_data_1.json\">Recevoir un fichier JSON</a><br>";

  response += "<a href=\"/gps_data\">Pousser le fichier JSON actuel vers l'application mobile</a><br>";
  response += "<a href=\"/get_json_data?value=date\">Obtenir une valeur spécifique du JSON actuel</a><br>";

  request->send(200, "text/html", response);
}



void handleListFiles(AsyncWebServerRequest *request) {
  Serial.println("Handling list files request");

  String filesList = "[";
  File root = SD.open("/");
  File file = root.openNextFile();
  while (file) {
    if (filesList != "[") {
      filesList += ",";
    }
    filesList += "\"" + String(file.name()) + "\"";
    file = root.openNextFile();
  }
  filesList += "]";

  request->send(200, "application/json", filesList);
}

void handlePointedGpsData(AsyncWebServerRequest *request) {
  if (!request->hasParam("file")) {
    request->send(400, "text/plain", "Missing 'file' parameter");
    return;
  }

  String filename = request->getParam("file")->value();
  Serial.println("Handling download request for file: " + filename);

  File file = SD.open(filename);
  if (!file) {
    request->send(404, "text/plain", "File not found");
    return;
  }

  request->send(SD, filename, "application/json");
}

void handleDownload(AsyncWebServerRequest *request) {
  if (!request->hasParam("file")) {
    request->send(400, "text/plain", "Missing 'file' parameter");
    return;
  }

  String filename = request->getParam("file")->value();
  Serial.println("Handling download request for file: " + filename);

  File file = SD.open(filename);
  if (!file) {
    request->send(404, "text/plain", "File not found");
    return;
  }

  request->send(SD, filename, "application/octet-stream");
}

void handleRemoveFile(AsyncWebServerRequest *request) {
  if (!request->hasParam("file")) {
    request->send(400, "text/plain", "Missing 'file' parameter");
    return;
  }

  String filename = request->getParam("file")->value();
  Serial.println("Handling remove file request for file: " + filename);

  if (SD.exists(filename)) {
    if (SD.remove(filename)) {
      request->send(200, "text/plain", "File removed successfully");
    } else {
      request->send(500, "text/plain", "Failed to remove file");
    }
  } else {
    request->send(404, "text/plain", "File not found");
  }
}

void handleGpsData(AsyncWebServerRequest *request) {


  File file = SD.open(DEFAULT_FILE);
  if (!file) {
    request->send(404, "text/plain", "File not found");
    return;
  }

  // Read JSON content from file
  size_t fileSize = file.size();
  if (fileSize > 1024) {
    request->send(500, "text/plain", "File too large");
    file.close();
    return;
  }

  // Allocate buffer for JSON content
  char jsonContent[fileSize + 1];
  memset(jsonContent, 0, sizeof(jsonContent));

  // Read JSON content from file
  size_t bytesRead = file.readBytes(jsonContent, fileSize);
  file.close();

  // Send JSON content as response
  request->send(200, "application/json", jsonContent);
}


void handleGpsValue(AsyncWebServerRequest *request) {
  // Ouvrir le fichier JSON
  File file = SD.open(DEFAULT_FILE);
  if (!file) {
    request->send(404, "text/plain", "Fichier non trouvé");
    return;
  }

  // Parse le contenu JSON
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, file);
  file.close();

  // Gérer les erreurs de parsing JSON
  if (error) {
    request->send(500, "text/plain", "Erreur lors de l'analyse du JSON");
    return;
  }

  // Vérifier si le tableau JSON contient un objet
  if (!doc.is<JsonArray>() || doc.size() == 0 || !doc[0].is<JsonObject>()) {
    request->send(500, "text/plain", "Le fichier JSON est invalide ou vide");
    return;
  }

  // Vérifier si le paramètre 'value' est présent dans la requête
  if (!request->hasParam("value")) {
    request->send(400, "text/plain", "Paramètre 'value' manquant");
    return;
  }

  // Récupérer la valeur demandée
  String valueName = request->getParam("value")->value();

  // Récupérer la valeur correspondante dans le premier objet JSON du tableau
  JsonObject obj = doc[0];
  if (obj.containsKey(valueName)) {
    // Si la valeur existe dans l'objet JSON, l'envoyer comme réponse
    String response = obj[valueName].as<String>();
    request->send(200, "text/plain", response);
  } else {
    // Si la valeur demandée n'existe pas dans l'objet JSON, renvoyer un message d'erreur
    request->send(404, "text/plain", "La valeur demandée n'existe pas dans le JSON");
  }
}
