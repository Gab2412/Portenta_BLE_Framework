#include <Arduino.h>
#include <ArduinoBLE.h>

// UUID casuali per il test
BLEService sismoService("19b10000-e8f2-537e-4f6c-d104768a1214");
// Specifichiamo 512 come dimensione massima della caratteristica
BLECharacteristic dataChar("19b10001-e8f2-537e-4f6c-d104768a1214", BLERead | BLENotify, 512);

void setup() {
  Serial.begin(115200);
  while (!Serial); // Attende il monitor seriale

  if (!BLE.begin()) {
    Serial.println("Errore nell'inizializzazione del Bluetooth!");
    while (1);
  }

  // Stampiamo il MAC per sicurezza
  Serial.print("Portenta avviata. MAC Address: ");
  Serial.println(BLE.address());

  BLE.setLocalName("Portenta_Sismo_Test");
  BLE.setAdvertisedService(sismoService);
  sismoService.addCharacteristic(dataChar);
  BLE.addService(sismoService);

  BLE.advertise();
  Serial.println("In attesa di connessione dall'ESP32...");
}

void loop() {
  BLEDevice central = BLE.central();

  if (central) {
    Serial.print("Connesso a central: ");
    Serial.println(central.address());

    while (central.connected()) {
      // Qui la Portenta non deve fare nulla. 
      // Risponderà automaticamente alle richieste di MTU dell'ESP32.
      delay(500);
    }

    Serial.println("Central disconnessa.");
  }
}