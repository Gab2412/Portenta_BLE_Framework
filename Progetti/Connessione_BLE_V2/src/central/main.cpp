#include <ArduinoBLE.h>

const char* serviceUUID = "19B10000-E8F2-537E-4F6C-D104768A1214";
const char* cmdUUID     = "19B10001-E8F2-537E-4F6C-D104768A1214";
const char* dataUUID    = "19B10002-E8F2-537E-4F6C-D104768A1214";

void setup() {
  Serial.begin(9600);
  if (!BLE.begin()) { while (1); }
  Serial.println("Central in scansione...");
  BLE.scanForUuid(serviceUUID);
}

void loop() {
  BLEDevice peripheral = BLE.available();

  if (peripheral) {
    BLE.stopScan();
    if (peripheral.connect()) {
      if (peripheral.discoverAttributes()) {
        BLECharacteristic cmdChar = peripheral.characteristic(cmdUUID);
        BLECharacteristic dataChar = peripheral.characteristic(dataUUID);
        
        dataChar.subscribe();
        Serial.println("Connesso! Digita 'r' per forzare una lettura.");

        while (peripheral.connected()) {
          // --- RICEZIONE DATI ---
          if (dataChar.valueUpdated()) {
            float valore;
            dataChar.readValue(&valore, sizeof(valore));
            Serial.print("Dato ricevuto: ");
            Serial.println(valore);
          }

          // --- INVIO COMANDO DA SERIALE ---
          if (Serial.available()) {
            char input = Serial.read();
            if (input == 'r' || input == 'R') {
              cmdChar.writeValue((byte)'R');
              Serial.println("Richiesta lettura anticipata inviata!");
            }
          }
        }
      }
    }
    Serial.println("Disconnesso. Riprendo scansione...");
    BLE.scanForUuid(serviceUUID);
  }
}