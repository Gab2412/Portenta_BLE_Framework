#include <ArduinoBLE.h>

BLEService myService("19b10000-e8f2-537e-4f6c-d104768a1214");
BLEStringCharacteristic textCommandChar("19b10001-e8f2-537e-4f6c-d104768a1214", BLERead | BLEWrite, 20);
BLEFloatCharacteristic temperatureChar("19b10002-e8f2-537e-4f6c-d104768a1214", BLERead | BLENotify);

void onBleConnected(BLEDevice central) {
  Serial.print("Connesso al central: ");
  Serial.println(central.address());
}

void onBleDisconnected(BLEDevice central) {
  Serial.print("Central disconnesso: ");
  Serial.println(central.address());
  BLE.advertise(); // Giustissimo rimetterlo in advertising qui!
  Serial.println("Advertising BLE riavviato.");
}

void setup() {
  Serial.begin(9600);
  unsigned long serialWaitStart = millis();
  while (!Serial && (millis() - serialWaitStart) < 3000) {}

  if (!BLE.begin()) {
    Serial.println("Errore avvio BLE!");
    while (1);
  }

  BLE.setEventHandler(BLEConnected, onBleConnected);
  BLE.setEventHandler(BLEDisconnected, onBleDisconnected);

  BLE.setDeviceName("Sensore");
  BLE.setLocalName("Sensore");
  BLE.setAdvertisedService(myService);

  myService.addCharacteristic(textCommandChar);
  myService.addCharacteristic(temperatureChar);
  BLE.addService(myService);

  textCommandChar.writeValue("");
  temperatureChar.writeValue(0.0);

  BLE.advertise(); // Avviato una sola volta
  Serial.println("Pronto! Advertising BLE attivo come 'Sensore'.");
}

void loop() {
  BLE.poll();

  BLEDevice central = BLE.central();

  if (central) {
    unsigned long lastSend = 0; // Timer per invio dati
    
    while (central.connected()) {
      BLE.poll(); // Necessario dentro il while
      
      // --- RICEZIONE TESTO ---
      if (textCommandChar.written()) {
        String comandoRicevuto = textCommandChar.value();
        Serial.print("Il central ha inviato: ");
        Serial.println(comandoRicevuto);
      }

      // --- INVIO TEMPERATURA (Ogni 1 secondo) ---
      if (millis() - lastSend > 1000) {
        float fintaTemperatura = 25.5 + (millis() % 1000) / 100.0; 
        temperatureChar.writeValue(fintaTemperatura);
        Serial.print("Inviata temp: ");
        Serial.println(fintaTemperatura);
        lastSend = millis();
      }
    }
  }
  // ELIMINATO: Il blocco else con l'advertise continuo che faceva crashare tutto.
}