String formatDMS(String input);
String formatTime(float timeDecimal);
void extractData_RMC(String data);
void extractData_GGA(String data);

void extractData_RMC(String data) {
  if (data.startsWith("$GNRMC")) {
    // Trame de données de vitesse
    // Séparer la trame en valeurs individuelles
    String values[13];
    int startIndex = 0;
    int commaIndex = 0;
    int valuesIndex = 0;
    while (commaIndex >= 0 && valuesIndex < 13) {
      commaIndex = data.indexOf(',', startIndex);
      if (commaIndex >= 0) {
        values[valuesIndex] = data.substring(startIndex, commaIndex);
        startIndex = commaIndex + 1;
        valuesIndex++;
      }
    }
    float altitude = values[9].toFloat();                                                                                                                           // Altitude en mètres
    String latitude = (valuesIndex >= 3 && values[3].length() > 0 && values[4].length() > 0) ? formatDMS(values[3] + " " + values[4]) : "null";   // Latitude en DMS ou "Null"
    String longitude = (valuesIndex >= 5 && values[5].length() > 0 && values[6].length() > 0) ? formatDMS(values[5] + " " + values[6]) : "null";  // Longitude en DMS ou "Null"
    String time = (valuesIndex >= 1) ? formatTime(values[1].toFloat()) : "null";                                                                                    // Heure UTC en HH:MM:SS
    // Extraire la vitesse
    float speedKmh = (valuesIndex >= 8 && values[7].length() > 0) ? values[7].toFloat() : 0.0;  // Vitesse en km/h
    String speed = (valuesIndex >= 8 && values[7].length() > 0) ? String(speedKmh) : "null";

    // Afficher les données sur le moniteur série
    Serial.print("Altitude: ");
    Serial.println((altitude != 0.0) ? String(altitude) : "null");

    Serial.print("Latitude: ");
    Serial.println(latitude);

    Serial.print("Longitude: ");
    Serial.println(longitude);

    Serial.print("Heure: ");
    Serial.println(time);

    Serial.print("Vitesse: ");
    Serial.print(speedKmh);
  }
}

void extractData_TXT(String data) {
  if (data.startsWith("$GNTXT")) {
    // Trame de données de texte
    // Afficher le texte brut
    Serial.println("Texte: " + data);
  }
}
void extractData_GGA(String data) {
  if (data.startsWith("$GNGGA")) {
    // Trame de données de localisation
    // Séparer la trame en valeurs individuelles
    String values[15];
    int startIndex = 0;
    int commaIndex = 0;
    int valuesIndex = 0;
    while (commaIndex >= 0 && valuesIndex < 15) {
      commaIndex = data.indexOf(',', startIndex);
      if (commaIndex >= 0) {
        values[valuesIndex] = data.substring(startIndex, commaIndex);
        startIndex = commaIndex + 1;
        valuesIndex++;
      }
    }

    // Extraire les données pertinentes
    float altitude = values[9].toFloat();                                                                                                         // Altitude en mètres
    String latitude = (valuesIndex >= 4 && values[2].length() > 0 && values[3].length() > 0) ? formatDMS(values[2] + " " + values[3]) : "null";   // Latitude en DMS ou "Null"
    String longitude = (valuesIndex >= 6 && values[4].length() > 0 && values[5].length() > 0) ? formatDMS(values[4] + " " + values[5]) : "null";  // Longitude en DMS ou "Null"
    String time = (valuesIndex >= 2) ? formatTime(values[1].toFloat()) : "null";                                                                  // Heure UTC ou "Null"



    // Afficher les données extraites
    Serial.print("Altitude: ");
    Serial.println((altitude != 0.0) ? String(altitude) : "null");  // Ou Null

    Serial.print("Latitude: ");
    Serial.println(latitude);

    Serial.print("Longitude: ");
    Serial.println(longitude);

    Serial.print("Heure: ");
    Serial.println(time);
  }
}

void setup() {
  Serial.begin(115200);

  // Exemple de trame GNRMC
  String GNRMC_Tram = "$GNGGA,142934.00,4538.65316,N,07350.59134,W,1,04,2.73,59.6,M,-32.8,M,,*43";
  // Exemple de trame GNGGA
  String GNGGA_Tram = "$GNRMC,142933.00,A,4538.65080,N,07350.59173,W,1.321,,270224,,,A*7B";

  // Exemple de coordonnées
  String latitude_input = "2503.6319 N";
  String longitude_input = "07136.0099 W";

  
  extractData_RMC(GNRMC_Tram);

  // Conversion de la latitude
  //Serial.println(formatDMS(latitude_input));

  // Conversion de la longitude
  //Serial.println(formatDMS(longitude_input));
}

void loop() {
  // Nothing to do here
}
