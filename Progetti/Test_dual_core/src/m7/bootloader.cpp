#include "Arduino.h"
#include "rtos.h" 

void setup() {
  bootM4(); //Avvia l'M4
}

void loop() {
  // Diciamo a M7 di attendere la flag 1 per poi addormentarsi "per sempre"
  rtos::ThisThread::flags_wait_any(1, osWaitForever); 
}

//In questo test, M7 avvia M4, il quale accenderò un led lampeggiante

//Caricare prima M4 e poi M7
