#include <SD.h>
#include <BluetoothSerial.h>

#define CHUNK_SIZE 128  // Taille du morceau de données à envoyer

BluetoothSerial SerialBT;
File file;  // Variable globale pour le fichier en cours de traitement

void setup() {
  Serial.begin(9600);
  SerialBT.begin("PerformChandail");  // Nom du dispositif Bluetooth
  SD.begin(5);                        // Pin CS pour la carte SD
  Serial.println("Bluetooth Start");
}

void loop() {
  if (SerialBT.available()) {
    String command = SerialBT.readStringUntil('\n');
    command.trim();

    if (command.equals("directoryfiles")) {
      listDirectory();
    } else if (command.startsWith("filespec:")) {
      command.remove(0, 9);  // Supprime "filespec:" du début de la chaîne
      command.trim();        // Supprime les espaces éventuels
      getFileDetails(command);
    } else if (command.startsWith("rmfile:")) {
      command.remove(0, 7);  // Supprime "rmfile:" du début de la chaîne
      command.trim();        // Supprime les espaces éventuels
      removeFile(command);
    } else if (command.startsWith("download:")) {
      command.remove(0, 9);   // Supprime "download:" du début de la chaîne
      command.trim();         // Supprime les espaces éventuels
      downloadFile(command);  // Appel de downloadFile avec le nom du fichier à télécharger
    } else if (command.equals("test")) {
      SerialBT.print(";Hello world;");
    } else {
      SerialBT.print(";");
      SerialBT.print("ERROR_COMMAND");
      SerialBT.print(";");
    }
  }
}

void listDirectory() {
  File root = SD.open("/", FILE_READ);
  if (!root) {
    SerialBT.print("ERROR_NO_REP_FOUND");
    SerialBT.print(";");
    Serial.println("ERROR_NO_FILE");
    return;
  }

  while (File entry = root.openNextFile()) {
    String filename = entry.name();
    SerialBT.print(";");
    SerialBT.print(filename);
    SerialBT.print(";");
    Serial.println(filename);
  }
  SerialBT.print("\n");
  root.close();
  Serial.println("Send directory list");
}

void getFileDetails(String filename) {
  Serial.print("File spec of ");
  Serial.println(filename);
  if (fileRead(filename)) {
    SerialBT.print(";");
    SerialBT.print("FileSpec");
    SerialBT.print(";");
    SerialBT.print("Name:");
    SerialBT.print(filename);
    SerialBT.print(";");
    SerialBT.print("Size:");
    SerialBT.print(file.size());
    SerialBT.print(";");
    SerialBT.print("PacketQTE:");
    SerialBT.print(numberPacket(filename));
    SerialBT.print(";");
    file.close();
  }
  SerialBT.print("\n");
  Serial.println("File detail send");
}

void downloadFile(String filename) {
  if (!fileRead(filename)) {
    SerialBT.print(";");
    SerialBT.print("ERROR_NO_FILE");
    SerialBT.print(";");
    return;
  }

  getFileDetails(filename);

  // Attendre une réponse du téléphone : "ACK"
  waitForACK();
  SerialBT.print(";");
  SerialBT.print("START_TRANSFER");
  SerialBT.print(";");
  Serial.println("START_TRANSFER");

  // Divise le fichier en paquets et les envoie un par un
  size_t packetNumber = 0;
  uint8_t buffer[CHUNK_SIZE];
  size_t totalPackets = numberPacket(filename);  // Déclaration et initialisation de totalPackets
  while (file.available()) {                     // Utilisation de file.available() sans argument
    // Lecture du chunk de données depuis le fichier
    size_t bytesRead = file.read(buffer, CHUNK_SIZE);
    // Envoi du numéro du paquet et de sa taille
    // Trame de check d'envoi : SEND;PN:*N° de paquet actuel*/*Nb de paquet total*;PS:*Taille du paquet*
    // PN : Packet Number                         // PS: Packet Size
    SerialBT.print(";");
    SerialBT.print("SEND;");
    SerialBT.print("PN:");
    SerialBT.print(++packetNumber);
    SerialBT.print("/");
    SerialBT.print(totalPackets);
    SerialBT.print(";");
    SerialBT.print("PS:");
    SerialBT.print(bytesRead);
    SerialBT.print(";");
    // Envoi du paquet
    SerialBT.print(";");
    SerialBT.write(buffer, bytesRead);
    SerialBT.print(";");
    // Attente de la confirmation du client de la réception du paquet :
    // Trame de check de réception type : ACK;PNR:*N° de paquet actuel*/*Nb de paquet total*;PSR:*Taille du paquet*
    // -> PNR : Packet Number received                // PSR: Packet Size Received
    // Attendre la confirmation du client de la réception du paquet
    waitForPacketACK(packetNumber, totalPackets, bytesRead);
  }
  SerialBT.print(";");
  SerialBT.print("TRANSFER_COMPLETE");
  SerialBT.print(";");
  Serial.println("TRANSFER_COMPLETE");
  file.close();
}

void waitForACK() {
  while (!SerialBT.available()) {
    // Attendre la réception de données
  }
  // Lire la réponse du client
  String response = SerialBT.readStringUntil('\n');
  response.trim();
  // Attendre la réponse "ACK" du téléphone
  while (response != "ACK") {
    // Attendre jusqu'à ce que "ACK" soit reçu
    response = SerialBT.readStringUntil('\n');
    response.trim();
  }
  Serial.println("DATA_ACKNOWLEDGED");
}

void waitForPacketACK(int packetNumber, int totalPackets, int bytesRead) {
  // Attendre la réception de données
  while (!SerialBT.available()) {
    // Attendre la réception de données
  }
  // Définir le format attendu de la réponse
  String expectedResponse = String("ACK;PNR:") + packetNumber + "/" + totalPackets + ";PSR:" + bytesRead;
  Serial.print("Expected reponse");
  Serial.print(expectedResponse);
  // Lire la réponse du client
  String response = SerialBT.readStringUntil('\n');
  response.trim();
  // Attendre la réponse de format spécifique du téléphone
  while (!response.equals(expectedResponse)) {
    // La trame ACK est mal formatée ou ne correspond pas à celle attendue
    Serial.println("Error: Invalid or unexpected acknowledgment frame received");
    SerialBT.print(";PACKET_CHECK_ERROR;");
    // Attendre la réception de données pour la prochaine tentative
    while (!SerialBT.available()) {
      // Attendre la réception de données
    }
    // Lire la réponse du client pour la prochaine tentative
    response = SerialBT.readStringUntil('\n');
    response.trim();
  }
}

void removeFile(String filename) {
  // Vérifie si le fichier existe
  if (!SD.exists(filename)) {
    SerialBT.print(";ERROR_NO_FILE;");
    Serial.print("ERROR_NO_FILE");
    return;
  }
  // Supprime le fichier s'il existe
  if (SD.remove(filename)) {
    SerialBT.print(";FILE_REMOVE_SUCCESS;");
    Serial.print("File removed :");
    Serial.println(filename);
  } else {
    SerialBT.print(";FILE_REMOVE_ERROR;");
    Serial.print("Error file removing");
  }
}

size_t numberPacket(String filename) {
  if (!fileRead(filename)) {  // Ajout de la condition pour appeler fileRead()
    return 0;                 // Retourner 0 si le fichier n'est pas trouvé
  }
  // Obtient la taille du fichier
  size_t fileSize = file.size();
  // Estime le nombre de morceaux
  size_t numChunks = fileSize / CHUNK_SIZE;
  if (fileSize % CHUNK_SIZE != 0) {
    numChunks++;  // Ajoute un morceau pour les octets restants
  }
  return numChunks;
}

bool fileRead(String filename) {
  file = SD.open(filename, FILE_READ);
  if (!file) {
    SerialBT.print(";Fichier non trouvé;");
    Serial.print("No file");
    return false;
  }
  return true;
}
