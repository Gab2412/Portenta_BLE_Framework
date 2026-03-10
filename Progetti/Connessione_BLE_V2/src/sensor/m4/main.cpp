#include <Arduino.h>

// ===== MEMORIA CONDIVISA CON M7 =====
// Stessi indirizzi usati dall'M7
volatile float* sharedTemp = (volatile float*)0x38000000;
volatile bool* dataReady = (volatile bool*)0x38000004;

void setup() {
  pinMode(LEDB, OUTPUT);
  digitalWrite(LEDB, LOW); // Blu ON = M4 avviato
  
  delay(2000); // Aspetta che M7 si stabilizzi
  
  pinMode(LEDR, OUTPUT);
  digitalWrite(LEDR, HIGH);
  
  // Inizializza memoria condivisa
  *sharedTemp = 0.0;
  *dataReady = false;
  
  // Lampeggio Blu = M4 pronto
  for(int i = 0; i < 3; i++) {
    digitalWrite(LEDB, HIGH);
    delay(150);
    digitalWrite(LEDB, LOW);
    delay(150);
  }
  
  // Setup completato - Blu fisso
  digitalWrite(LEDB, LOW);
}

void loop() {
  // Simula lettura sensore (temperatura fittizia)
  static unsigned long lastSend = 0;
  
  if (millis() - lastSend > 10000) { // Manda un dato ogni 10 secondi
    float fakeTemp = 24.0 + (random(0, 100) / 10.0); // 24.0 - 34.0°C
    
    // Scrivi la temperatura in memoria condivisa
    *sharedTemp = fakeTemp;
    *dataReady = true;
    
    // Lampeggio rosso = Dato inviato
    digitalWrite(LEDR, LOW);
    delay(50);
    digitalWrite(LEDR, HIGH);
    
    lastSend = millis();
  }
  
  delay(10);
}