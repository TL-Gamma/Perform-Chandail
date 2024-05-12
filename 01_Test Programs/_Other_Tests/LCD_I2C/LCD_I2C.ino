// GPIO 21 : SDA
// GPIO 22 : SCL

#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);  // set the LCD address to 0x27 for a 16 chars and 2 line display

void setup() {

  float voltage = 0.0;
  double lat = 0.0;
  double lng = 0.0;
  String Hour = "\0";
  String mode = "\0";
  LCD_config();
  LCD_displayMode(mode);
  LCD_displayVoltage(voltage);
  LCD_displayLat(lat);
  LCD_displayLng(lng);
  LCD_displayHour(Hour);
}

void loop() {
}


void LCD_config() {
  lcd.init();       // Initialisation de l'écran LCD
  lcd.clear();      // Effacement de l'écran LCD
  lcd.backlight();  // Activation du rétroéclairage de l'écran LCD
  lcd.setCursor(0, 0);
  lcd.print("Mode:");
  lcd.setCursor(12, 0);
  lcd.print("| U:");
  lcd.setCursor(0, 1);
  lcd.print("Lat:");
  lcd.setCursor(0, 2);
  lcd.print("Lng:");
  lcd.setCursor(0, 3);
  lcd.print("Hour:");
}

void LCD_displayVoltage(float value) {
  lcd.setCursor(16, 0);  // Positionnement du curseur à la deuxième ligne, onzième caractère
  lcd.print(value, 2);   // Affichage de la tension
}
void LCD_displayMode(String value) {
  lcd.setCursor(5, 0);  // Positionnement du curseur à la deuxième ligne, onzième caractère
  if (value == "\0") {
    lcd.print("null");
  } else {
    lcd.print(value);  // Affichage de la tension
  }
}

void LCD_displayLat(double value) {
  lcd.setCursor(5, 1);  // Positionnement du curseur à la deuxième ligne, onzième caractère
  lcd.print(value, 8);  // Affichage de la tension
}

void LCD_displayLng(double value) {
  lcd.setCursor(5, 2);  // Positionnement du curseur à la deuxième ligne, onzième caractère
  lcd.print(value, 8);  // Affichage de la tension
}

void LCD_displayHour(String value) {
  lcd.setCursor(6, 3);  // Positionnement du curseur à la deuxième ligne, onzième caractère
    if (value == "\0") {
    lcd.print("null");
  } else {
    lcd.print(value);  // Affichage de la tension
  }
}