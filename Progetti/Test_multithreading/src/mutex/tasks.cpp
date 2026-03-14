#include "tasks.h"
#include <chrono>

using namespace std::chrono;

// Definiamo le risorse
Mutex dataMutex;
int32_t sharedCounter = 0;
bool newDataAvailable = false;

void consumerTask() {
    while (true) {
        // 1. Tenta di prendere il lucchetto
        dataMutex.lock();

        // 2. Se il produttore ha messo un nuovo dato...
        if (newDataAvailable) {
            Serial.print("[CONSUMATORE] Ho prelevato il dato: ");
            Serial.println(sharedCounter);
            newDataAvailable = false; // Diciamo che l'abbiamo letto
        }

        // 3. Rilascia SEMPRE il lucchetto il prima possibile
        dataMutex.unlock();

        // Controlla ogni 500ms (più veloce del produttore)
        ThisThread::sleep_for(500ms);
    }
}