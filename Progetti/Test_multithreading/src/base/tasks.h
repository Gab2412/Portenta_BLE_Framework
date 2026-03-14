#ifndef TASKS_H
#define TASKS_H

#include <Arduino.h>
#include "rtos.h"

// Usiamo il namespace di Mbed per i Thread e i Mutex
using namespace rtos;

// Dichiariamo le funzioni che diventeranno Thread
void blinkTask();
void loggerTask();

// Dichiariamo i puntatori ai Thread (così main.cpp può vederli)
extern Thread threadBlink;
extern Thread threadLogger;

#endif