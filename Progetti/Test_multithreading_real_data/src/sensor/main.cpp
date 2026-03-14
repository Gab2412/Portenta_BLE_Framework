#include <Arduino.h>
#include "tasks.h"
#include "seismic_data.h"

// Variabili locali al loop (non condivise)
int32_t localBuffer[3][10];
int sampleIndex = 0;
int dataPointer = 0;
unsigned long lastSample = 0;

void setup() {
    Serial.begin(115200);
    delay(2000);

    // Facciamo partire il thread della comunicazione
    threadBLE.start(bleTask);

    // Facciamo partire il thread del lampeggio
    threadBlink.start(blinkTask);
    
    Serial.println("Sistema Sismico Multi-thread avviato...");
}

void loop() {
    // 100 Hz (10ms)
    if (millis() - lastSample >= 10) {
        lastSample = millis();

        // Lettura dati (simulata da file .h o reale da sensore)
        for (int ch = 0; ch < 3; ch++) {
            localBuffer[ch][sampleIndex] = SEISMIC_SAMPLES[ch][dataPointer];
        }

        sampleIndex++;
        dataPointer++;
        if (dataPointer >= SEISMIC_DATA_LEN) dataPointer = 0;

        // Ogni 10 campioni (1 pacchetto pronto)
        if (sampleIndex >= 10) {
            uint8_t tempPacket[66];
            int pos = 0;

            // --- DELTA ENCODING ---
            for (int ch = 0; ch < 3; ch++) {
                int32_t firstVal = localBuffer[ch][0];
                memcpy(&tempPacket[pos], &firstVal, 4); pos += 4;
                for (int i = 1; i < 10; i++) {
                    int16_t delta = (int16_t)(localBuffer[ch][i] - localBuffer[ch][i-1]);
                    memcpy(&tempPacket[pos], &delta, 2); pos += 2;
                }
            }

            // --- PASSAGGIO DATI AL THREAD BLE ---
            dataMutex.lock();
            memcpy(sharedPacket, tempPacket, 66);
            packetReady = true;
            dataMutex.unlock();

            sampleIndex = 0;
        }
    }
}