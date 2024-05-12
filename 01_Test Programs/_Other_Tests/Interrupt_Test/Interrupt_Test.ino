#define BUTTON_PIN 15     // Broche du bouton poussoir
#define INTEGRATED_LED 2  // Integrated LED PIN

bool bpState = false;  // Variable booléenne pour indiquer si l'enregistrement est en cours

void setup() {
  Serial.begin(115200);                                                        // Serial Monitor
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonPressed, FALLING);  // Interrupt Pin
  pinMode(INTEGRATED_LED, OUTPUT)
}

void loop() {
  // LED Blinking
  digitalWrite(INTEGRATED_LED, true);
  delay(1000);
  digitalWrite(INTEGRATED_LED, false);
  delay(1000);
}

// fonction de l'interruption
void buttonPressed() {
  bpState = !bpState;  // Inversion de l'état d'enregistrement
  if (bpState) {
    Serial.print("Button State is true");
  } else {
    Serial.print("Button State is false");
  }
}