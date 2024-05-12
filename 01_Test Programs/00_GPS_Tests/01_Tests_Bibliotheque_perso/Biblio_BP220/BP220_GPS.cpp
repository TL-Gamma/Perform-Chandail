/********************************************************************************************
Patch Notes : 

A faire : 
  -> Implementer les fonctions de formatage 
    | Fonctionalité de verification du format d'entré avant le formatage
********************************************************************************************/


#include "BP220_GPS.h"

BP220_GPS::BP220_GPS(uint8_t rxPin, uint8_t txPin)
  : _serial(1) {
  _serial.begin(9600, SERIAL_8N1, rxPin, txPin);
}



bool BP220_GPS::begin() {
  delay(100);  // Attente pour la stabilisation du module
  return true;
}

/**
 * @brief Lit une trame NMEA selon le mode spécifié.
 * 
 * @param mode Le mode de lecture de la trame NMEA ("ALL", "DEFAULT", "GGA", ou "RMC").
 *             - "ALL": Lit toutes les trames NMEA commençant par "$".
 *             - "DEF": Lit les trames NMEA "$GNGGA" ou "$GNRMC".
 *             - "GGA": Lit uniquement les trames NMEA "$GNGGA".
 *             - "RMC": Lit uniquement les trames NMEA "$GNRMC".
 * @return La première trame NMEA qui correspond au mode spécifié, ou une chaîne vide si aucune trame correspondante n'est trouvée.
 */


//////////////////////////////// Fonction Permettant de lire une trame NMEA ////////////////////////////////

String BP220_GPS::readNMEA(String mode) {
  String nmea;
  while (_serial.available()) {
    nmea = _serial.readStringUntil('\n');  // Lire une ligne de données
    if (mode == "ALL") {
      if (nmea.startsWith("$")) {
        return nmea;
      }
    } else if (mode == "DEFAULT") {
      if (nmea.startsWith("$GNGGA") || nmea.startsWith("$GNRMC")) {
        updateGGAPositionData(nmea);
        updateRMCPositionData(nmea);
        return nmea;
      }
    } else if (mode == "GGA") {
      if (nmea.startsWith("$GNGGA")) {
        updateGGAPositionData(nmea);
        return nmea;
      }
    } else if (mode == "RMC") {
      if (nmea.startsWith("$GNRMC")) {
        updateRMCPositionData(nmea);
        return nmea;
      }
    }
  }
}


//////////////////////////////// Fonction interne permettant d'extraire sous la forme d'un tableau dynamique les données de la trame NMEA et remplire  ////////////////////////////////


std::vector<String> BP220_GPS::extractNMEA(String data) {
  std::vector<String> values;
  int startIndex = 0;
  int commaIndex = 0;

  // Extraire les valeurs en séparant la chaîne par les virgules
  while (startIndex < data.length()) {
    commaIndex = data.indexOf(',', startIndex);
    if (commaIndex >= 0) {
      values.push_back(data.substring(startIndex, commaIndex));
      startIndex = commaIndex + 1;
    } else {
      // Si aucune virgule n'est trouvée, cela signifie que nous sommes à la dernière valeur
      // Nous prenons donc la sous-chaîne restante
      values.push_back(data.substring(startIndex));
      break;
    }
  }

  // Remplir les cases vides avec la chaîne "null"
  for (size_t i = 0; i < values.size(); i++) {
    if (values[i].isEmpty()) {
      values[i] = "null";
    }
  }

//////////////////////////////// Si la trame est une trame RMC, extraire les données et les stocker dans la structure RMCData ////////////////////////////////
  if (values[0] == "$GNRMC") {
    _rmcData.speed = values[7];
    _rmcData.date = values[8];
    _rmcData.time = values[1];
    _rmcData.latitude = values[3];
    _rmcData.indicLat = values[4]; 
    _rmcData.longitude = values[5];
    _rmcData.indicLng = values[6]; 
  }

//////////////////////////////// Si la trame est une trame GGA, extraire les données et les stocker dans la structure GGAData ////////////////////////////////
  if (values[0] == "$GNGGA") {
    _ggaData.altitude = values[8];
    _ggaData.time = values[1];
    _ggaData.latitude = values[2];
    _ggaData.indicLat = values[3];
    _ggaData.longitude = values[4];
    _ggaData.indicLng = values[5];
  }

  return values;
}


//////////////////////////////// Afficher le tableau de valeur (-> Fonction de debug) ////////////////////////////////
void BP220_GPS::printSeparatedValues(String data) {
  // Extraire les valeurs de la trame NMEA
  std::vector<String> values = extractNMEA(data);

  // Afficher les valeurs extraites
  Serial.println("Valeurs extraites de la trame NMEA :");
  for (size_t i = 0; i < values.size(); i++) {
    Serial.print("Valeur ");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(values[i]);
  }
}

//////////////////////////////// Fonction pour mettre à jour les données GGA à partir de la trame NMEA ////////////////////////////////
void BP220_GPS::updateGGAPositionData(String data) {
  // Extraire les données pertinentes de la trame NMEA et les mettre à jour dans la structure _ggaData
  std::vector<String> values = extractNMEA(data);

  _ggaData.altitude = values[0];
  _ggaData.time = values[1];
  _ggaData.latitude = values[2];
  _ggaData.indicLat = values[3];
  _ggaData.longitude = values[4];
  _ggaData.indicLng = values[5];
}

//////////////////////////////// Fonction pour mettre à jour les données RMC à partir de la trame NMEA ////////////////////////////////
void BP220_GPS::updateRMCPositionData(String data) {
  // Extraire les données pertinentes de la trame NMEA et les mettre à jour dans la structure _rmcData
  std::vector<String> values = extractNMEA(data);

  _rmcData.speed = values[0];
  _rmcData.date = values[1];
  _rmcData.time = values[2];
  _rmcData.latitude = values[3];
  _rmcData.indicLat = values[4];
  _rmcData.longitude = values[5];
  _rmcData.indicLng = values[6];
}




////////////Fonctions Retun RMC ////////////////
String BP220_GPS::RMCget_Speed() {
  return  _rmcData.speed;
}
String BP220_GPS::RMCget_Date() {
  return  _rmcData.date;
}

String BP220_GPS::RMCget_Time() {
  return  _rmcData.time;
}
String BP220_GPS::RMCget_Latitude() {
  return  _rmcData.latitude;
}
String BP220_GPS::RMCget_indicLat() {
  return  _rmcData.indicLat;
}
String BP220_GPS::RMCget_longitude() {
  return  _rmcData.longitude;
}
String BP220_GPS::RMCget_indicLng() {
  return  _rmcData.indicLng;
}

////////////Fonctions Retun GGA ////////////////

String BP220_GPS::GGAget_Altitude() {
  return  _ggaData.altitude;
}
String BP220_GPS::GGAget_Time() {
  return  _ggaData.time;
}
String BP220_GPS::GGAget_Latitude() {
  return  _ggaData.latitude;
}
String BP220_GPS::GGAget_indicLat() {
  return  _ggaData.indicLat;
}
String BP220_GPS::GGAget_longitude() {
  return  _ggaData.longitude;
}
String BP220_GPS::GGAget_indicLng() {
  return  _ggaData.indicLng;
}
