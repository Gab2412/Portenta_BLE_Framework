#ifndef TASKS_H
#define TASKS_H

#include <Arduino.h>
#include "rtos.h"

using namespace rtos; // ci permette di scrivere semplicemente Thread e non rtos::Thread

// Funzioni dei Thread
void consumerTask();

// Risorse condivise (extern per farle vedere a tutti i file)
extern Mutex dataMutex;         // Il lucchetto
extern int32_t sharedCounter;   // La nostra "finta" lettura sensore
extern bool newDataAvailable;   // Un flag per dire "ehi, c'è un nuovo dato!"

#endif