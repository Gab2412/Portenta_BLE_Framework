#include <ArduinoBLE.h>

BLEService seismicService("19b10000-e8f2-537e-4f6c-d104768a1214");
BLECharacteristic dataChar("19b10001-e8f2-537e-4f6c-d104768a1214", BLERead | BLENotify, 70);

const int samplesPerPacket = 10;
int32_t buffer[3][samplesPerPacket];
int sampleIndex = 0;

// Variabili per simulazione sismica
float eventSignal = 0;
unsigned long lastEventTime = 0;

void setup() {
  Serial.begin(115200);
  if (!BLE.begin()) while (1);
  BLE.setLocalName("Sismometro_M7");
  BLE.setAdvertisedService(seismicService);
  seismicService.addCharacteristic(dataChar);
  BLE.addService(seismicService);
  BLE.advertise();
}

void loop() {
  static unsigned long lastSample = 0;
  if (millis() - lastSample >= 10) { // 100 Hz
    lastSample = millis();

    // 1. Simuliamo un evento ogni 10 secondi
    if (millis() - lastEventTime > 10000) {
      eventSignal = 8000.0; // Picco iniziale
      lastEventTime = millis();
    }
    
    // 2. Onda smorzata (il sismo che decade) + Rumore di fondo
    eventSignal *= 0.98; // Smorzamento esponenziale
    float seismicWave = eventSignal * sin(2.0 * PI * 2.5 * (millis() / 1000.0)); // 2.5 Hz

    for (int ch = 0; ch < 3; ch++) {
      float noise = random(-50, 50); // Rumore di fondo realistico
      buffer[ch][sampleIndex] = (int32_t)(seismicWave + noise);
    }

    sampleIndex++;
    if (sampleIndex >= samplesPerPacket) {
      uint8_t packet[70];
      int pos = 0;
      for (int ch = 0; ch < 3; ch++) {
        int32_t firstVal = buffer[ch][0];
        memcpy(&packet[pos], &firstVal, 4); pos += 4;
        for (int i = 1; i < samplesPerPacket; i++) {
          int16_t delta = (int16_t)(buffer[ch][i] - buffer[ch][i-1]);
          memcpy(&packet[pos], &delta, 2); pos += 2;
        }
      }
      dataChar.writeValue(packet, pos);
      sampleIndex = 0;
    }
  }
  BLE.poll();
}