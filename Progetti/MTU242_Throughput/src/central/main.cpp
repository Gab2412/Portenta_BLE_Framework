#include <Arduino.h>
#include <NimBLEDevice.h>

std::string targetAddress = "a8:61:0a:41:65:7f";
unsigned long startTime = 0;
int packetCount = 0;

class ClientCallbacks : public NimBLEClientCallbacks {
    void onConnect(NimBLEClient* pClient) override {
        pClient->exchangeMTU();
    }
    void onDisconnect(NimBLEClient* pClient, int reason) override {
        Serial.println("Disconnesso.");
    }
};

// Funzione che parte quando la Portenta invia un dato. Calcola la velocità
void notifyCB(NimBLERemoteCharacteristic* pRemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
    if (packetCount == 0) startTime = millis(); // Inizia a contare dal primo pacchetto
    packetCount++;

    if (packetCount >= 20) {
        unsigned long duration = millis() - startTime;
        Serial.printf(">>> BENCHMARK: Ricevuti 20 pacchetti (4840 byte) in %lu ms\n", duration);
        Serial.printf(">>> VELOCITÀ REALE: %.2f KB/s\n", (4.840 / (duration / 1000.0)));
        packetCount = 0; // Reset per il prossimo burst
    }
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    NimBLEDevice::init("ESP32_Benchmark");
    NimBLEDevice::setMTU(517);
}

void loop() {
    static NimBLEClient* pClient = NimBLEDevice::createClient();
    
    if (!pClient->isConnected()) {
        pClient->setClientCallbacks(new ClientCallbacks(), true);
        if (pClient->connect(NimBLEAddress(targetAddress, 0))) {
            Serial.println("Connesso!");
            
            // Troviamo la caratteristica e attiviamo le notifiche
            auto pService = pClient->getService("19b10000-e8f2-537e-4f6c-d104768a1214");
            if (pService) {
                auto pChar = pService->getCharacteristic("19b10001-e8f2-537e-4f6c-d104768a1214");
                if (pChar && pChar->canNotify()) {
                    pChar->subscribe(true, notifyCB);
                }
            }
        }
    } else {
        Serial.printf("MTU: %d | In attesa di dati...\n", pClient->getMTU());
        delay(2000);
    }
}