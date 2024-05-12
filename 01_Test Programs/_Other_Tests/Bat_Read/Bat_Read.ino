// GPIO 21 : SDA
// GPIO 22 : SCL
#include <LiquidCrystal_I2C.h>
// Définition des constantes
#define INTERUPTOR_PIN 34  // Utiliser le numéro de broche approprié pour votre ESP32
#define VOLTAGE_DIVIDER_FACTOR 2.3 // Facteur de multiplication de la tension pour retrouver la tension avant le pont diviseur
#define BATTERY_LOW_THRESHOLD 3.6 // Limite minimale de tension pour préserver la batterie
// Initialisation de l'écran LCD
LiquidCrystal_I2C lcd(0x27, 20, 4);  // Configuration de l'adresse de l'écran à 0x27 pour un affichage 16 caractères sur 2 lignes
// Fonction pour lire le niveau de la batterie
float readBattLevel() {
  int sensorValue = analogRead(INTERUPTOR_PIN);  // Lecture de la tension analogique
  float voltage = (sensorValue * (3.3 / 4095) * VOLTAGE_DIVIDER_FACTOR);  // Conversion de la valeur analogique en tension
  return voltage;
}
// Configuration initiale
void setup() {
  Serial.begin(115200);  // Initialisation de la communication série
  lcd.init();  // Initialisation de l'écran LCD
  lcd.clear();  // Effacement de l'écran LCD
  lcd.backlight();  // Activation du rétroéclairage de l'écran LCD
}
// Boucle principale
void loop() {
  float voltage = readBattLevel();  // Lecture de la tension de la batterie
  if (voltage > 0) {  // Si la batterie a de la tension
    Serial.println("Mode course");  // Affichage du mode dans la console série
    lcd.setCursor(0, 0);  // Positionnement du curseur à la première ligne, premier caractère
    lcd.print("Mode course");  // Affichage du mode sur l'écran LCD
    if (voltage < BATTERY_LOW_THRESHOLD) {  // Si la tension de la batterie est inférieure au seuil de préservation de la batterie
      // Mettre l'ESP32 en veille profonde si la tension est trop faible pour acquérir des données
      //lcd.clear();  // Effacement de l'écran LCD
      //lcd.print("Sleep");  // Affichage de "Sleep" sur l'écran LCD
      esp_deep_sleep_start();  // Mise en veille profonde de l'ESP32
    }
  } else {  // Sinon (si la tension de la batterie est inférieure ou égale à 0.5V)
    Serial.println("Mode BT");  // Affichage du mode dans la console série
    lcd.setCursor(0, 0);  // Positionnement du curseur à la première ligne, premier caractère
    lcd.print("Mode BT     ");  // Affichage du mode sur l'écran LCD
  }
  // Affichage de la tension de la batterie dans la console série
  Serial.print("Tension : ");
  Serial.println(voltage);
  // Affichage de la tension de la batterie sur l'écran LCD
  lcd.setCursor(0, 1);  // Positionnement du curseur à la deuxième ligne, premier caractère
  lcd.print("Tension : ");
  lcd.setCursor(10, 1);  // Positionnement du curseur à la deuxième ligne, onzième caractère
  lcd.print(voltage); // Affichage de la tension
  delay(500);  // Attente de 500 millisecondes avant la prochaine lecture de la tension de la batterie
}