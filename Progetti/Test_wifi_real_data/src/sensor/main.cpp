#include <ArduinoBLE.h>
#include "seismic_data.h" // Il nuovo header con array [3][actual_len]

// Servizio e Caratteristica (66 byte totali necessari per 3 canali compressi)
BLEService seismicService("19b10000-e8f2-537e-4f6c-d104768a1214");
BLECharacteristic dataChar("19b10001-e8f2-537e-4f6c-d104768a1214", BLERead | BLENotify, 70);

const int samplesPerPacket = 10;
int32_t buffer[3][samplesPerPacket];
int sampleIndex = 0;
int dataPointer = 0; 

void setup() {
  Serial.begin(115200);
  // while (!Serial); // Decommenta se vuoi attendere l'apertura del monitor seriale

  if (!BLE.begin()) {
    Serial.println("Errore Hardware BLE!");
    while (1);
  }

  BLE.setLocalName("Sismometro_M7");
  BLE.setAdvertisedService(seismicService);
  seismicService.addCharacteristic(dataChar);
  BLE.addService(seismicService);
  BLE.advertise();

  Serial.println("--- SENSOR READY ---");
  Serial.print("Streaming dati REALI (3 canali). Campioni: ");
  Serial.println(SEISMIC_DATA_LEN);
}

void loop() {
  static unsigned long lastSample = 0;
  
  // Gestione connessione: il replay avanza solo se qualcuno ascolta
  BLEDevice central = BLE.central();

  if (central && central.connected()) {
    if (millis() - lastSample >= 10) { // Frequenza 100 Hz
      lastSample = millis();

      // Lettura dei 3 canali reali dall'header
      for (int ch = 0; ch < 3; ch++) {
        buffer[ch][sampleIndex] = SEISMIC_SAMPLES[ch][dataPointer];
      }

      sampleIndex++;
      dataPointer++;

      // Gestione Loop Infinito dell'array
      if (dataPointer >= SEISMIC_DATA_LEN) {
        dataPointer = 0;
        Serial.println("\n[LOOP] Fine dati. Ricomincio...");
      }

      // Impacchettamento e Compressione ogni 10 campioni
      if (sampleIndex >= samplesPerPacket) {
        uint8_t packet[70];
        int pos = 0;

        for (int ch = 0; ch < 3; ch++) {
          // 1. Valore base a 32 bit (4 byte) per il canale attuale
          int32_t firstVal = buffer[ch][0];
          memcpy(&packet[pos], &firstVal, 4); 
          pos += 4;

          // 2. 9 Delta a 16 bit (18 byte) per il canale attuale
          for (int i = 1; i < samplesPerPacket; i++) {
            int16_t delta = (int16_t)(buffer[ch][i] - buffer[ch][i-1]);
            memcpy(&packet[pos], &delta, 2); 
            pos += 2;
          }
        }

        // Invio del pacchetto (totale 66 byte)
        dataChar.writeValue(packet, pos);
        sampleIndex = 0;

        // Feedback seriale leggero
        if (dataPointer % 1000 == 0) Serial.print("."); 
      }
    }
  } else {
    // Reset se la connessione cade, per ripartire dall'inizio del file
    if (dataPointer != 0) {
      dataPointer = 0;
      sampleIndex = 0;
      Serial.println("\nCentral disconnessa. Reset puntatore.");
    }
  }

  BLE.poll();
}