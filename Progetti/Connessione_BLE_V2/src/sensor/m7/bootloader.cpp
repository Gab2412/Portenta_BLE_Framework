#include <Arduino.h>
#include <ArduinoBLE.h>

// ===== SERVIZIO E CARATTERISTICHE BLE =====
BLEService myService("19b10000-e8f2-537e-4f6c-d104768a1214");
BLEStringCharacteristic textCommandChar("19b10001-e8f2-537e-4f6c-d104768a1214", BLERead | BLEWrite, 20);
BLEFloatCharacteristic temperatureChar("19b10002-e8f2-537e-4f6c-d104768a1214", BLERead | BLENotify);

// ===== MEMORIA CONDIVISA CON M4 =====
// Indirizzo memoria D3 SRAM condivisa tra M7 e M4
volatile float* sharedTemp = (volatile float*)0x38000000;
volatile bool* dataReady = (volatile bool*)0x38000004;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("M7: Inizializzazione - GESTORE BLE");
  
  pinMode(LEDB, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDR, OUTPUT);
  digitalWrite(LEDB, LOW);  // Blu ON durante setup
  digitalWrite(LEDG, HIGH);
  digitalWrite(LEDR, HIGH);
  
  // Inizializza memoria condivisa
  *sharedTemp = 0.0;
  *dataReady = false;
  
  // Inizializza BLE
  Serial.println("M7: Inizializzazione BLE...");
  if (!BLE.begin()) {
    Serial.println("M7: ERRORE BLE!");
    digitalWrite(LEDR, LOW); // Rosso fisso = errore
    while (1);
  }
  Serial.println("M7: BLE inizializzato");
  
  // Configura servizio e caratteristiche
  BLE.setDeviceName("Sensore");
  BLE.setLocalName("Sensore");
  BLE.setAdvertisedService(myService);
  
  myService.addCharacteristic(textCommandChar);
  myService.addCharacteristic(temperatureChar);
  BLE.addService(myService);
  
  textCommandChar.writeValue("");
  temperatureChar.writeValue(0.0);
  
  BLE.advertise();
  Serial.println("M7: Advertising BLE attivo");
  
  // Avvia M4
  delay(500);
  Serial.println("M7: Avvio M4...");
  bootM4();
  Serial.println("M7: M4 avviato");
  
  // Verde ON = Setup completato
  digitalWrite(LEDB, HIGH);
  digitalWrite(LEDG, LOW);
}

void loop() {
  // Polling BLE continuo
  BLE.poll();
  
  // Controlla se M4 ha inviato nuovi dati
  if (*dataReady) {
    float temp = *sharedTemp;
    
    if (BLE.connected()) {
      temperatureChar.writeValue(temp);
      Serial.print("M7: Temperatura aggiornata: ");
      Serial.println(temp);
    }
    
    *dataReady = false; // Reset flag
  }
  
  // Gestione comandi ricevuti dal central
  if (textCommandChar.written()) {
    String cmd = textCommandChar.value();
    Serial.print("M7: Comando ricevuto: ");
    Serial.println(cmd);
  }
  
  delay(10);
}