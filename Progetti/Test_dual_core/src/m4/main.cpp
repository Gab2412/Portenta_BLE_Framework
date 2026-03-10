#include "Arduino.h"

void setup() {
  // Inizializziamo il LED Verde (G = Green)
  pinMode(LEDG, OUTPUT);
}

void loop() {
  // Se il LED si accende significa che il core M4 è partito correttamente
  digitalWrite(LEDG, LOW);  // Acceso (logica inversa)
  delay(200);
  digitalWrite(LEDG, HIGH); // Spento
  delay(200);
}