#include <Arduino.h>
#include <ArduinoBLE.h>

BLEService benchmarkService("19b10000-e8f2-537e-4f6c-d104768a1214");
// Caratteristica impostata esattamente sull'MTU negoziato (242 byte)
BLECharacteristic dataChar("19b10001-e8f2-537e-4f6c-d104768a1214", BLERead | BLENotify, 242);

uint8_t dummyData[242];

void setup() {
  Serial.begin(115200);
  for(int i=0; i<242; i++) dummyData[i] = i; // Dati di prova

  if (!BLE.begin()) {
    Serial.println("Errore BLE!");
    while (1);
  }

  BLE.setLocalName("Portenta_Benchmark");
  BLE.setAdvertisedService(benchmarkService);
  benchmarkService.addCharacteristic(dataChar);
  BLE.addService(benchmarkService);
  BLE.advertise();
  
  Serial.println("Portenta pronta per il benchmark velocità...");
}

void loop() {
  BLEDevice central = BLE.central();
  if (central) {
    while (central.connected()) {
      // Invio di 20 pacchetti ogni 2 secondi
      Serial.println("Inizio Burst di 20 pacchetti...");
      for(int i=0; i<20; i++) {
        dummyData[0] = i; // Cambiamo il primo byte per distinguere i pacchetti
        dataChar.writeValue(dummyData, 242);
      }
      Serial.println("Burst completato. Attesa...");
      delay(2000); 
    }
  }
}