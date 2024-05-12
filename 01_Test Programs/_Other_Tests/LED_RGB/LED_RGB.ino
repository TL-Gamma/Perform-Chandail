//Common to +3.3V et  penser a tester au multimètre !!!


#include <Arduino.h>

#define PINLED_R 27 
#define PINLED_B 26
#define PINLED_G 25

// Constantes de couleurs
// R , V , B
#define WHITE 128, 255, 255
#define RED 255, 0, 0
#define GREEN 0, 255, 0
#define BLUE 0, 0, 255
#define CYAN 0, 255, 255
#define MAGENTA 128, 0, 255
#define YELLOW 128, 255, 0


void setColor(uint8_t redValue, uint8_t greenValue, uint8_t blueValue);

void setup() {
  pinMode(PINLED_R, OUTPUT);
  pinMode(PINLED_G, OUTPUT);
  pinMode(PINLED_B, OUTPUT);
  Serial.begin(9600);

}

void loop() {
  
  ledBlink(WHITE);  // WHITE
  Serial.println("WHITE");

  ledBlink(RED);  // RED
  Serial.println("RED");
  
  ledBlink(GREEN);  // GREEN
  Serial.println("GREEN");
  ledBlink(BLUE);  // BLUE
  Serial.println("BLUE");



  ledBlink(MAGENTA);  // MAGENTA
  Serial.println("MAGENTA");
  ledBlink(CYAN);  // CYAN
  Serial.println("CYAN");
  ledBlink(YELLOW);  // YELLOW
  Serial.println("YELLOW");
  /*
  setColor(255, 0, 0);  // Red
  Serial.println("Red");
  delay(500);
  setColor(0, 0, 0);  // None
  delay(500);
  setColor(0, 255, 0);  // Green
  Serial.println("Green");
  delay(500);
  setColor(0, 0, 0);  // None
  delay(500);
  setColor(0, 0, 255);  // Blue
  Serial.println("Blue");
  delay(500);
  setColor(0, 0, 0);  // Red
  delay(500);
  */
}

void setColor(uint8_t redValue, uint8_t greenValue, uint8_t blueValue) {
  redValue = 255 - redValue;
  greenValue = 255 - greenValue;
  blueValue = 255 - blueValue;
  analogWrite(PINLED_R, redValue);
  analogWrite(PINLED_G, greenValue);
  analogWrite(PINLED_B, blueValue);
}

void ledBlink(uint8_t redValue, uint8_t greenValue, uint8_t blueValue) {
  setColor(redValue, greenValue, blueValue);
  delay(1000);
  ledOff();
  delay(1000);
}

void ledOff() {
  analogWrite(PINLED_R, 255);
  analogWrite(PINLED_G, 255);
  analogWrite(PINLED_B, 255);
}
