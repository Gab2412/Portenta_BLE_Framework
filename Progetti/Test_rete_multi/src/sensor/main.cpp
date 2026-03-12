#include <ArduinoBLE.h>
#include "seismic_data.h" 

// Definizioni UUID
#define SERVICE_UUID        "19b10000-e8f2-537e-4f6c-d104768a1214"
#define CHARACTERISTIC_UUID "19b10001-e8f2-537e-4f6c-d104768a1214"

BLEService seismicService(SERVICE_UUID);
BLECharacteristic dataChar(CHARACTERISTIC_UUID, BLERead | BLENotify, 70);

// Gestione ID dinamico da PlatformIO
#ifndef SENSOR_ID
  #define SENSOR_ID 0 
#endif

const int samplesPerPacket = 10;
int32_t buffer[3][samplesPerPacket];
int sampleIndex = 0;
int dataPointer = 0; 
char localName[20];

void setup() {
  Serial.begin(115200);
  
  if (!BLE.begin()) {
    Serial.println("Errore Hardware BLE!");
    while (1);
  }

  // Generazione nome dinamico: Sismometro_01, Sismometro_02, ecc.
  sprintf(localName, "Sismometro_%02d", SENSOR_ID);
  
  BLE.setLocalName(localName);
  BLE.setAdvertisedService(seismicService);
  seismicService.addCharacteristic(dataChar);
  BLE.addService(seismicService);
  BLE.advertise();

  Serial.print("--- SENSORE "); Serial.print(SENSOR_ID); Serial.println(" AVVIATO ---");
  Serial.print("Nome BLE: "); Serial.println(localName);
}

void loop() {
  static unsigned long lastSample = 0;
  BLEDevice central = BLE.central();

  if (central && central.connected()) {
    if (millis() - lastSample >= 10) { // 100 Hz
      lastSample = millis();

      // Lettura dati reali dall'array bidimensionale
      for (int ch = 0; ch < 3; ch++) {
        buffer[ch][sampleIndex] = SEISMIC_SAMPLES[ch][dataPointer];
      }

      sampleIndex++;
      dataPointer++;

      if (dataPointer >= SEISMIC_DATA_LEN) dataPointer = 0; // Loop infinito

      if (sampleIndex >= samplesPerPacket) {
        uint8_t packet[66]; // 3 canali * (4 base + 18 delta) = 66 byte
        int pos = 0;

        for (int ch = 0; ch < 3; ch++) {
          int32_t firstVal = buffer[ch][0];
          memcpy(&packet[pos], &firstVal, 4); pos += 4;
          for (int i = 1; i < samplesPerPacket; i++) {
            int16_t delta = (int16_t)(buffer[ch][i] - buffer[ch][i-1]);
            memcpy(&packet[pos], &delta, 2); pos += 2;
          }
        }
        dataChar.writeValue(packet, 66);
        sampleIndex = 0;
      }
    }
  } else {
    if (dataPointer != 0) {
      dataPointer = 0;
      sampleIndex = 0;
      Serial.println("In attesa di connessione...");
    }
  }
  BLE.poll();
}