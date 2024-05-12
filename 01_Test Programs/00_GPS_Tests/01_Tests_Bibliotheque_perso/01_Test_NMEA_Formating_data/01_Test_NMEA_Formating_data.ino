


#define COEF_KTS_KMH 1.852

String formatTime(float timeDecimal);
String formatDate(int inputDate);
String formatDMS(String input);
String formatDD(String input);
void extractData_RMC(String data);
void extractData_GGA(String data);
String* separateNMEA(String data, String separator, int& valuesCount); // Ajout de la déclaration de la fonction separateNMEA

/*************************************************/
// Fonctions utilitaires //

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

/************************************************/

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


String formatTime(String inputTime) {
  // Extraire les parties heures, minutes et secondes de la chaîne d'entrée
  int hours = inputTime.substring(0, 2).toInt();
  int minutes = inputTime.substring(2, 4).toInt();
  int seconds = inputTime.substring(4, 6).toInt();

  // Formater l'heure en HH:MM:SS
  return String(hours) + ":" + (minutes < 10 ? "0" : "") + String(minutes) + ":" + (seconds < 10 ? "0" : "") + String(seconds);
}

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



void setup() {
  Serial.begin(115200);

  // Exemple de trame GNRMC
  String GNRMC_Tram = "$GPRMC,053740.000,A,2503.6319,N,12136.0099,E,2.69,79.65,100106,,,A*53";
  // Exemple de trame GNGGA
  String GNGGA_Tram = "$$GPGGA,064036.289,4836.5375,N,00740.9373,E,1,04,3.2,200.2,M,,,,0000*0E";

  // Exemple de coordonnées
  String latitude_input = "2503.6319 N";
  String longitude_input = "07136.0099 W";

  extractData(GNRMC_Tram);
  //extractData_GGA(GNGGA_Tram);
/*
  int valuesCount;
  String* values = separateNMEA(GNRMC_Tram, ",", valuesCount);

  Serial.println("Valeurs extraites :");
  printValues(values, valuesCount);
  */
  // Conversion de la latitude
  //Serial.println(formatDMS(latitude_input));

  // Conversion de la longitude
  //Serial.println(formatDMS(longitude_input));
}

void loop() {
  // Nothing to do here
}
