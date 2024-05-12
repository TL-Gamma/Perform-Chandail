#define boutonPin 15 // Définir le numéro de broche utilisé pour le bouton
#define seuilPressionLongue 1000 // Définir la durée seuil pour une pression longue en millisecondes
unsigned long tempsDebutPression = 0; // Variable pour enregistrer le temps au début de la pression

void setup() {
  pinMode(boutonPin, INPUT_PULLUP); // Configurer la broche du bouton en entrée avec résistance de pull-up interne
  Serial.begin(9600); // Initialiser la communication série pour le débogage
}

void loop() {
  // Lire l'état du bouton
  int etatBouton = digitalRead(boutonPin);

  // Si le bouton est enfoncé
  if (etatBouton == LOW) {
    // Si c'est la première fois que le bouton est enfoncé, enregistrer le temps de début de pression
    if (tempsDebutPression == 0) {
      tempsDebutPression = millis();
    }

    // Vérifier si la durée de la pression dépasse le seuil pour une pression longue
    if (millis() - tempsDebutPression >= seuilPressionLongue) {
      // Une pression longue a été détectée
      Serial.println("Pression longue détectée !");
      // Ajoutez ici le code pour exécuter l'action associée à la pression longue
      // Réinitialiser le temps de début de pression pour éviter la répétition des actions
      tempsDebutPression = 0;
    }
  } else {
    // Le bouton est relâché, réinitialiser le temps de début de pression
    tempsDebutPression = 0;
  }

  // Ajoutez ici d'autres instructions de votre programme
}
