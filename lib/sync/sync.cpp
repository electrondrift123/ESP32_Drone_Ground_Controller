#include <Arduino.h>
#include "sync.h"

SemaphoreHandle_t serialMutex;

bool mutexes_init(void){
    serialMutex = xSemaphoreCreateMutex();
    if (serialMutex == NULL) {
        Serial.println("Failed to create serial mutex!");
        return false;
        while (1); // halt or retry
    }

    return true;
}