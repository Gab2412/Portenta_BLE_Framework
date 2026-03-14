#include <Arduino.h>
#include "tasks.h"

Thread threadConsumer;

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("--- TEST MUTEX: PRODUTTORE/CONSUMATORE ---");

    // Facciamo partire solo il consumatore
    threadConsumer.start(consumerTask);
}

void loop() {
    // SIMULAZIONE ACQUISIZIONE (Il Produttore)
    static int32_t fintoSensore = 0;
    fintoSensore += 10; // Generiamo un dato

    Serial.println("--- [PRODUTTORE] Genero nuovo dato... ---");

    // Entriamo nella sezione critica
    dataMutex.lock();
    
    sharedCounter = fintoSensore; // Scriviamo il dato
    newDataAvailable = true;      // Alziamo la bandierina
    
    dataMutex.unlock();

    // Il produttore è lento: genera dati ogni 2 secondi
    delay(2000);
}