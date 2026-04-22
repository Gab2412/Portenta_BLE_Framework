#include <Arduino.h>
#include <NimBLEDevice.h>

static NimBLEClient* pClient = nullptr;
// Il MAC address della tua Portenta
std::string targetAddress = "a8:61:0a:41:65:7f"; 

class ClientCallbacks : public NimBLEClientCallbacks {
    void onConnect(NimBLEClient* pClient) override {
        Serial.println(">>> Connesso! Richiedo negoziazione...");
        // In NimBLE 2.5.0 la funzione non accetta parametri
        pClient->exchangeMTU();
    }

    void onDisconnect(NimBLEClient* pClient, int reason) override {
        Serial.printf(">>> Disconnesso (Motivo: %d)\n", reason);
    }
    
    // NOTA: onMTUChange rimosso perché non supportato in 2.5.0 per i Client
};

void setup() {
    Serial.begin(115200);
    delay(2000); 
    Serial.println("=== ESP32 GATEWAY TEST MTU (NimBLE 2.5.0) ===");

    NimBLEDevice::init("ESP32_Gateway_MTU");

    // Impostiamo l'MTU desiderato GLOBALMENTE
    NimBLEDevice::setMTU(517); 
}

void loop() {
    if (pClient == nullptr) {
        pClient = NimBLEDevice::createClient();
        pClient->setClientCallbacks(new ClientCallbacks(), true);
    }

    if (!pClient->isConnected()) {
        Serial.println("Ricerca della Portenta...");
        
        /**
         * CORREZIONE INDIRIZZO:
         * NimBLE 2.5.0 richiede esplicitamente il tipo di indirizzo.
         * 0 corrisponde a BLE_ADDR_TYPE_PUBLIC (quello della Portenta).
         */
        NimBLEAddress addr(targetAddress, 0); 
        
        if (pClient->connect(addr)) {
            Serial.println("Connessione riuscita!");
        } else {
            Serial.println("Connessione fallita. Riprovo...");
            delay(5000);
        }
    } else {
        // Monitoriamo l'MTU direttamente leggendolo dal client ogni 2 secondi
        uint16_t currentMTU = pClient->getMTU();
        Serial.printf("Stato: CONNESSO | MTU Attuale: %d\n", currentMTU);
        
        if (currentMTU > 23) {
            Serial.println(">>> SUCCESSO: La negoziazione ha funzionato!");
        } else {
            Serial.println(">>> ATTESA: MTU ancora al valore di base...");
        }
        delay(2000);
    }
}