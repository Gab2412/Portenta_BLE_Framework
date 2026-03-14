#include "tasks.h"
#include <chrono> // Necessario per usare ms, s, ecc.

using namespace std::chrono; // Per scrivere 150ms invece di std::chrono::milliseconds(150)

// Definiamo i Thread qui
Thread threadBlink;
Thread threadLogger;

// --- TASK 1: Lampeggio LED ---
void blinkTask() {
    pinMode(LEDG, OUTPUT); 
    while (true) {
        digitalWrite(LEDG, LOW);  // Acceso (Logica inversa)
        ThisThread::sleep_for(150ms); 
        digitalWrite(LEDG, HIGH); // Spento
        ThisThread::sleep_for(150ms);
    }
}

// --- TASK 2: Logger Seriale ---
void loggerTask() {
    int count = 0;
    while (true) {
        Serial.print("[INFO] Thread Logger attivo. Ciclo: ");
        Serial.println(count++);
        // Usiamo 3s per indicare 3 secondi
        ThisThread::sleep_for(3s); 
    }
}