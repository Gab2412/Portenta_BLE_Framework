#include <ArduinoBLE.h>
#include "rtos.h"

BLEService myService("19B10000-E8F2-537E-4F6C-D104768A1214");
BLEByteCharacteristic commandChar("19B10001-E8F2-537E-4F6C-D104768A1214", BLEWrite);
BLEFloatCharacteristic dataChar("19B10002-E8F2-537E-4F6C-D104768A1214", BLERead | BLENotify);

unsigned long lastSendTime = 0;
const unsigned long interval = 10000; // 10 secondi

void setup() {
  // NOTA: La Serial su M4 non è collegata direttamente all'USB.
  // Usiamo i LED per il debug.
  pinMode(LEDB, OUTPUT);   // LED Blu
  digitalWrite(LEDB, HIGH); // Spento (logica inversa)

  if (!BLE.begin()) {
    while (1) { // Errore: lampeggio veloce rosso se possibile, o resta qui
      digitalWrite(LEDB, LOW); delay(100); digitalWrite(LEDB, HIGH); delay(100);
    }
  }

  BLE.setLocalName("Sensore_M4");
  BLE.setAdvertisedService(myService);
  myService.addCharacteristic(commandChar);
  myService.addCharacteristic(dataChar);
  BLE.addService(myService);
  
  BLE.advertise();

  // Lampo Blu lungo: Setup completato con successo
  digitalWrite(LEDB, LOW); delay(1000); digitalWrite(LEDB, HIGH);
}

void effettuaLetturaEInvia() {
  // Feedback visivo: il LED Blu lampeggia quando invia dati
  digitalWrite(LEDB, LOW); 
  
  float fintaTemp = 22.0 + (random(0, 50) / 10.0);
  dataChar.writeValue(fintaTemp);
  
  delay(20); // Brevissimo tempo per la stabilità
  digitalWrite(LEDB, HIGH); 
}

void loop() {
  BLE.poll();

  BLEDevice central = BLE.central();

  if (central && central.connected()) {
    unsigned long currentMillis = millis();

    // 1. Invio Periodico (ogni 10s)
    if (currentMillis - lastSendTime >= interval) {
      effettuaLetturaEInvia();
      lastSendTime = currentMillis;
    }

    // 2. Invio su Comando (se riceve 'R')
    if (commandChar.written()) {
      if (commandChar.value() == 'R') {
        effettuaLetturaEInvia();
        lastSendTime = currentMillis;
      }
    }
    
    // Piccolo sleep RTOS per non saturare la CPU
    rtos::ThisThread::sleep_for(10); 
  } else {
    // Se non connesso, dormi più a lungo
    rtos::ThisThread::sleep_for(100);
  }
}

