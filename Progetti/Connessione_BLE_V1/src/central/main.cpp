#include <ArduinoBLE.h>

const char* serviceUUID = "19b10000-e8f2-537e-4f6c-d104768a1214";
const char* textUUID    = "19b10001-e8f2-537e-4f6c-d104768a1214";
const char* tempUUID    = "19b10002-e8f2-537e-4f6c-d104768a1214";

void setup() {
  Serial.begin(9600);
  unsigned long serialWaitStart = millis();
  while (!Serial && (millis() - serialWaitStart) < 3000) {}

  if (!BLE.begin()) {
    Serial.println("Errore nell'avvio del BLE sulla Scheda 2!");
    while (1);
  }

  Serial.println("Scheda Centrale avviata. Cerco il servizio specifico...");
  // FONDAMENTALE: Cerca SOLO dispositivi che offrono l'UUID specificato
  BLE.scan(); 
}

bool connettiEGestisci(BLEDevice peripheral) {
  Serial.println("Trovato! Provo a connettermi...");

  if (!peripheral.connect()) {
    Serial.println("Connessione fallita.");
    return false;
  }

  if (!peripheral.discoverAttributes()) {
    Serial.println("Impossibile scoprire le caratteristiche.");
    peripheral.disconnect();
    return false;
  }

  BLECharacteristic textChar = peripheral.characteristic(textUUID);
  BLECharacteristic tempChar = peripheral.characteristic(tempUUID);

  if (!textChar || !tempChar) {
    Serial.println("Errore: UUID non corrispondenti.");
    peripheral.disconnect();
    return false;
  }

  if (!tempChar.subscribe()) {
    Serial.println("Attenzione: subscribe temperatura non riuscita.");
  }
  
  Serial.println("Connessione stabilita con successo!");
  Serial.println("Scrivi un messaggio nel Monitor Seriale e premi INVIO.");

  while (peripheral.connected()) {
    // 1. LEGGI SERIALE -> INVIA ALLA SCHEDA 1
    if (Serial.available()) {
      String msg = Serial.readStringUntil('\n');
      msg.trim();
      if (msg.length() > 0) {
        // Modo più sicuro per inviare stringhe via BLE
        textChar.writeValue((byte*)msg.c_str(), msg.length());
        Serial.print("Inviato: ");
        Serial.println(msg);
      }
    }

    // 2. RICEVI TEMPERATURA DALLA SCHEDA 1
    if (tempChar.valueUpdated()) {
      float t;
      tempChar.readValue(&t, sizeof(t));
      Serial.print("Temperatura ricevuta: ");
      Serial.print(t);
      Serial.println(" °C");
    }
  }

  Serial.println("La Scheda 1 si è disconnessa.");
  return true;
}

void loop() {
  BLEDevice peripheral = BLE.available();

  if (peripheral) {
    // Filtro manuale: controlliamo se il nome corrisponde a "Sensore"
    if (peripheral.localName() == "Sensore") {
      Serial.println("Trovato dispositivo 'Sensore'! Fermo la scansione...");
      BLE.stopScan(); 
      
      connettiEGestisci(peripheral);
      
      Serial.println("Ritorno in scansione...");
      BLE.scan();
    }
  }
}