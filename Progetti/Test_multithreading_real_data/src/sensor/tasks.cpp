#include "tasks.h"
#include <chrono>

using namespace std::chrono;

// Definiamo le risorse
Thread threadBLE;
Mutex dataMutex;
uint8_t sharedPacket[66];
bool packetReady = false;

// UUID e Servizi
BLEService seismicService("19b10000-e8f2-537e-4f6c-d104768a1214");
BLECharacteristic dataChar("19b10001-e8f2-537e-4f6c-d104768a1214", BLERead | BLENotify, 70);

// --- Task BLE ---
void bleTask() {
    if (!BLE.begin()) {
        Serial.println("Errore Hardware BLE!");
        return;
    }

    char localName[20];
    sprintf(localName, "Sismometro_%02d", SENSOR_ID);
    BLE.setLocalName(localName);
    BLE.setAdvertisedService(seismicService);
    seismicService.addCharacteristic(dataChar);
    BLE.addService(seismicService);
    BLE.advertise();

    Serial.print("BLE Thread Avviato per: "); Serial.println(localName);

    while (true) {
        BLEDevice central = BLE.central();

        if (central && central.connected()) {
            // Se c'è un pacchetto pronto, lo spediamo
            dataMutex.lock();
            if (packetReady) {
                dataChar.writeValue(sharedPacket, 66);
                packetReady = false; // Pacchetto inviato
            }
            dataMutex.unlock();
        }
        
        BLE.poll();
        // Un piccolo respiro per il thread
        ThisThread::sleep_for(5ms); 
    }
}

// --- Task Blink ---
Thread threadBlink; //Definisco il thread

void blinkTask() {
    pinMode(LEDG, OUTPUT); 
    while (true) {
        digitalWrite(LEDG, LOW);  // Acceso (Logica inversa)
        ThisThread::sleep_for(200ms); 
        digitalWrite(LEDG, HIGH); // Spento
        ThisThread::sleep_for(200ms);
    }
}