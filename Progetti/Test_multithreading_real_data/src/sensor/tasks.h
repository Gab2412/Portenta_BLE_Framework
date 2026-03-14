#ifndef TASKS_H
#define TASKS_H

#include <Arduino.h>
#include <ArduinoBLE.h>
#include "rtos.h"

using namespace rtos;

// --- CONFIGURAZIONE SENSOR ID (da PlatformIO) ---
#ifndef SENSOR_ID
  #define SENSOR_ID 0 
#endif

// --- DICHIARAZIONI ---
void bleTask();             // La funzione del thread BLE
extern Thread threadBLE;    // L'oggetto thread
extern Mutex dataMutex;     // Il lucchetto per i dati

// --- RISORSE CONDIVISE ---
extern uint8_t sharedPacket[66]; // Il pacchetto compresso pronto per l'invio
extern bool packetReady;         // La bandierina per il thread BLE

// --- Thread Blink ---
void blinkTask();
extern Thread threadBlink;

#endif