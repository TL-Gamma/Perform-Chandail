#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SD.h>
#include <ArduinoJson.h>

#define DEFAULT_FILE "/gps_data.json"

const char *ssid = "PerformChandail";
const char *password = "12345678";

AsyncWebServer server(80);

void setup() {
  Serial.begin(115200);

  // Connect to WiFi
  WiFi.softAP(ssid, password);
  Serial.println("WiFi connected");
  Serial.print("Wifi Name :");
  Serial.println(ssid);
  Serial.print("PWD :");
  Serial.println(password);

  // Print the IP address of the server
  Serial.print("Server IP address: ");
  Serial.println(WiFi.softAPIP());

  // Initialize SD card
  if (!SD.begin()) {
    Serial.println("Failed to initialize SD card");
    return;
  }
  Serial.println("SD card initialized");

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

void loop() {
}

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
