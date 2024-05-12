#include <HardwareSerial.h>

#define GPS_SERIAL_RX 16  // Connecter RX du GPS à la broche 16 de l'ESP32
#define GPS_SERIAL_TX 17  // Connecter TX du GPS à la broche 17 de l'ESP32
#define GPS_BAUDRATE 9600

#define COEF_KTS_KMH 1.852

#define EXTRACT_DATA_ENABLED true  // Définir à true pour extraire les données, false pour simplement les lire

HardwareSerial gpsSerial(1);  // Utiliser le port série matériel 1

// Prototypes
String formatTime(String inputTime);
String formatDate(String inputDate);
String formatDMS(String input);
String formatDD(String input);
void extractData_RMC(String data);
void extractData_GGA(String data);
String* separateNMEA(String data, String separator, int& valuesCount); // Ajout de la déclaration de la fonction separateNMEA




/**********************************************FORMATAGE DATE****************************************************/

String formatDate(String inputDate) {
  // Vérifier si la chaîne de caractères a une longueur valide (6 caractères)
  if (inputDate.length() != 6) {
    return "Invalid input"; // Retourner un message d'erreur si la longueur est incorrecte
  }

  // Extraire les parties jour, mois et année de la chaîne de caractères
  int day = inputDate.substring(0, 2).toInt();
  int month = inputDate.substring(2, 4).toInt();
  int year = 2000 + inputDate.substring(4).toInt();

  // Formater la date au format "YYYY-MM-DD"
  return String(year) + "-" + (month < 10 ? "0" : "") + String(month) + "-" + (day < 10 ? "0" : "") + String(day);
}

/**********************************************FORMATAGE HEURE****************************************************/
String formatTime(String inputTime) {
  // Extraire les parties heures, minutes et secondes de la chaîne d'entrée
  int hours = inputTime.substring(0, 2).toInt();
  int minutes = inputTime.substring(2, 4).toInt();
  int seconds = inputTime.substring(4, 6).toInt();

  // Formater l'heure en HH:MM:SS
  return String(hours) + ":" + (minutes < 10 ? "0" : "") + String(minutes) + ":" + (seconds < 10 ? "0" : "") + String(seconds);
}

/**********************************************FORMATAGE LOCALISATION EN DMS****************************************************/

String formatDMS(String input) {
  double coord;
  char direction;
  int n = sscanf(input.c_str(), "%lf %c", &coord, &direction);

  if (n == 2) {
    int degrees = (int)(coord / 100);
    int minutes = (int)coord - (degrees * 100);
    double seconds = (coord - degrees * 100 - minutes) * 60;

    return String(degrees) + "° " + String(minutes) + "' " + String(seconds, 3) + "\" " + direction;
  } else {
    return "Invalid input";
  }
}
/**********************************************FORMATAGE LOCALISATION EN DD****************************************************/

String formatDD(String input) {
  double coord;
  int n = sscanf(input.c_str(), "%lf", &coord);

  if (n == 1) {
    int degrees = (int)(coord / 100);
    double minutes = coord - (degrees * 100);
    double decimalDegrees = degrees + (minutes / 60.0);
    return String(decimalDegrees, 6);  // Le deuxième argument spécifie le nombre de décimales à afficher
  } else {
    // En cas d'entrée invalide
    return "Invalid input";
  }
}


/*******************************************SEPARATION DES DONNES DE LA TRAME NMEA*********************************************/
///////////////////////////////UTILITAIRE D'AFFICHAGE DES DONNEES DE LA FONCTION SUIVANTE /////////////////////////////////////////
void printValues(String* values, int valuesCount) {
  if (values == NULL) {
    Serial.println("Aucune valeur n'a été extraite.");
    return;
  }

  for (int i = 0; i < valuesCount; i++) {
    Serial.print("values[");
    Serial.print(i);
    Serial.print("] = ");
    Serial.println(values[i]);
  }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

String* separateNMEA(String data, String separator, int& valuesCount) {
  int startIndex = 0;
  int commaIndex = 0;
  int valuesIndex = 0;
  int maxValues = (data.startsWith("$GPGGA")) ? 17 : 15; // GGA a 17 valeurs, RMC a 15 valeurs
  String* values = new String[maxValues];

  while (valuesIndex < maxValues) {
    commaIndex = data.indexOf(separator, startIndex);
    if (commaIndex >= 0) {
      values[valuesIndex] = data.substring(startIndex, commaIndex);
      startIndex = commaIndex + 1;
    } else {
      // Si aucune virgule n'est trouvée, cela signifie que nous sommes à la dernière valeur
      // Nous prenons donc la sous-chaîne restante
      values[valuesIndex] = data.substring(startIndex);
      break; // Sortir de la boucle
    }
    valuesIndex++;
  }

  // Si aucune valeur n'a été extraite, retourner NULL
  if (valuesIndex == 0 && commaIndex < 0) {
    delete[] values; // Libérer la mémoire allouée
    values = NULL;
  }

  valuesCount = valuesIndex + 1; // Ajouter 1 pour la dernière valeur
  return values;
}

/************************************************EXTRACTION DES DONNEES D'UNE TRAME GNRMC******************************************/
void extractData_RMC(String data) {
  // Déclaration des variables pour stocker les données extraites
  String date;
  String time;
  String latitude;
  String longitude;
  float F_speedKnots;
  float F_speedKmph;
  String speed;
  // Séparation des valeurs de la trame NMEA
  int valuesCount;
  String* values = separateNMEA(data, ",", valuesCount);

  if (values != NULL && valuesCount >= 12) { // Vérification que des valeurs ont été extraites et qu'il y a suffisamment de valeurs
    // Extraction des données spécifiques
    date = formatDate(values[9]); // Date
    time = formatTime(values[1]); // Heure
    latitude = formatDMS(values[3] + " " + values[4]); // Latitude
    longitude = formatDMS(values[5] + " " + values[6]); // Longitude
    F_speedKnots = values[7].toFloat(); // Vitesse

    // convertion vitesse knots vers km/h
    F_speedKmph = F_speedKnots*COEF_KTS_KMH;
    speed = String(F_speedKmph);

    // Affichage des données extraites
    Serial.println("Données extraites pour GNRMC :");
    Serial.println("Date : " + date);
    Serial.println("Heure : " + time);
    Serial.println("Latitude : " + latitude);
    Serial.println("Longitude : " + longitude);
    Serial.println("Vitesse : " + speed);


    delete[] values; // Libération de la mémoire allouée
  } else {
    Serial.println("Erreur lors de l'extraction des données GNRMC.");
  }
}

/*******************************************EXTRACTION DES DONNEES D'UNE TRAME GNGGA******************************************************/

void extractData_GGA(String data) {
  // Déclaration des variables pour stocker les données extraites
  String time;
  String latitude;
  String dirLat;
  String longitude;
  String dirLong;
  String altitude;

  // Séparation des valeurs de la trame NMEA
  int valuesCount;
  String* values = separateNMEA(data, ",", valuesCount);

  if (values != NULL && valuesCount >= 14) { // Vérification que des valeurs ont été extraites et qu'il y a suffisamment de valeurs
   
    time = formatTime(values[1]); // Heure
    latitude = values[2] ; // Latitude
    dirLat = values[3];
    longitude = values[4]; // Longitude
    dirLong =  values[5];
    altitude = values[9]; // Altitude
    delete[] values; // Libération de la mémoire allouée
  } else {
    Serial.println("Erreur lors de l'extraction des données GNGGA.");
  }
}
void setup() {
  Serial.begin(115200);
  gpsSerial.begin(GPS_BAUDRATE, SERIAL_8N1, GPS_SERIAL_RX, GPS_SERIAL_TX);  // Initialise la communication avec le module GPS  
  delay(1000);                                                              // Attendre un court instant pour que le module GPS soit prêt
}

void loop() {
  while (gpsSerial.available()) {
    String dataLine = gpsSerial.readStringUntil('\n');  // Lire une ligne de données
#if EXTRACT_DATA_ENABLED
     
#else
    Serial.println(dataLine);  // Afficher les données sur le moniteur série de l'ESP32
#endif
  }
}






