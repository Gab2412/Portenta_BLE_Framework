#include "Arduino.h"
#include "rtos.h" 

void setup() {
  // 1. Avvia l'M4
  bootM4(); 
}

void loop() {
  // 2. Mbed OS 6: Diciamo al thread dell'M7 di aspettare che venga 
  // alzata la "bandierina" numero 1. 
  // Usando osWaitForever, il sistema operativo capisce che non c'è un timeout,
  // quindi addormenta l'M7 per sempre.
  rtos::ThisThread::flags_wait_any(1, osWaitForever); 
}