#include <Arduino.h>
#include "tasks.h"

void setup() {
    Serial.begin(115200);
    delay(2000); 
    Serial.println("--- SISTEMA MULTI-THREAD AVVIATO ---");

    // Facciamo partire i thread
    // Passiamo la funzione che devono eseguire
    threadBlink.start(blinkTask);
    threadLogger.start(loggerTask);
}

void loop() {
    // Il loop principale (Thread Main) è libero.
    // Qui andrà il tuo codice "intoccabile" che legge i sensori.
    Serial.println("--> Loop principale (Sensore) in esecuzione...");
    delay(10000); // Anche un delay enorme qui non bloccherà gli altri thread
}