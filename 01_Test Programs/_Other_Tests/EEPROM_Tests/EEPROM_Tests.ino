#include <EEPROM.h>
#define EEPROM_ADDRESS 0    
void setup() {
  Serial.begin(115200);

  // Initialiser l'émulation EEPROM
  EEPROM.begin(512);

  String filename = "GPS_data_98.json";
  // Écrire une chaîne de caractères dans l'EEPROM
  writeStringToEEPROM(EEPROM_ADDRESS, filename);

  // Lire la chaîne de caractères depuis l'EEPROM pour vérification
  String storedString = readStringFromEEPROM(EEPROM_ADDRESS);
  Serial.print("Chaîne lue depuis l'EEPROM : ");
  Serial.println(storedString);

  // Fermer l'émulation EEPROM
  EEPROM.end();
}

void loop() {
  // Ce code ne sera exécuté qu'une seule fois au démarrage
}

void writeStringToEEPROM(int addr, const String &str) {
  int len = str.length();
  
  // Vérifier si la chaîne de caractères rentre dans l'EEPROM
  if (len > EEPROM.length() - 1) {
    Serial.println("Erreur : la chaîne de caractères est trop longue pour l'EEPROM.");
    return;
  }
  
  // Écrire la longueur de la chaîne de caractères en premier
  EEPROM.write(addr, len);
  
  // Écrire chaque caractère de la chaîne de caractères
  for (int i = 0; i < len; ++i) {
    EEPROM.write(addr + 1 + i, str[i]);
  }
}

String readStringFromEEPROM(int addr) {
  // Lire la longueur de la chaîne de caractères
  int len = EEPROM.read(addr);
  
  // Lire chaque caractère et construire la chaîne de caractères
  String str = "";
  for (int i = 0; i < len; ++i) {
    str += char(EEPROM.read(addr + 1 + i));
  }
  
  return str;
}
