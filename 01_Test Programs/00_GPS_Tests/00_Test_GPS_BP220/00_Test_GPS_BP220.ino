#include <HardwareSerial.h>

#define GPS_SERIAL_RX 16  // Connecter RX du GPS à la broche 16 de l'ESP32
#define GPS_SERIAL_TX 17  // Connecter TX du GPS à la broche 17 de l'ESP32
#define GPS_BAUDRATE 9600


HardwareSerial gpsSerial(1);  // Utiliser le port série matériel 1



void setup() {
  Serial.begin(115200);
  gpsSerial.begin(GPS_BAUDRATE, SERIAL_8N1, GPS_SERIAL_RX, GPS_SERIAL_TX);  // Initialise la communication avec le module GPS
  delay(1000);                                                              // Attendre un court instant pour que le module GPS soit prêt

  // Configuration du module GPS
  //configureGPS();
}

void loop() {

  while (gpsSerial.available()) {
    char c = gpsSerial.read();
    Serial.print(c);  // Afficher les données sur le moniteur série de l'ESP32
  }
}

void configureGPS() {  // Cette configuration n'est pas utile
  Serial.println("Début Configuration");

  // Configuration du débit de données
  gpsSerial.println("$PUBX,41,1,9600,0,0,0*0A");

  delay(100);  // Attendre un court instant entre les commandes

  // Configuration pour rechercher uniquement les signaux GPS
  gpsSerial.println("$PUBX,40,GPS,0,0,0,0*5A");
  delay(100);  // Attendre un court instant entre les commandes

  // Activation des messages GGA pour obtenir des données de positions
  gpsSerial.println("$PUBX,00,GGA,1,1,0*04");
  delay(100);  // Attendre un court instant entre les commandes

  // Configuration de la fréquence de mise à jour des données à 1 Hz
  gpsSerial.println("$PUBX,08,1*15");
  delay(100);  // Attendre un court instant entre les commandes

  // Sauvegarde des paramètres
  gpsSerial.println("$PUBX,43*");
  delay(100);  // Attendre un court instant entre les commandes

  Serial.println("Fin Configuration");
}
