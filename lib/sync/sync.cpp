#include <Arduino.h>
#include "sync.h"

SemaphoreHandle_t serialMutex;
// SemaphoreHandle_t wdtMutex;

extern portMUX_TYPE wdtMutex = portMUX_INITIALIZER_UNLOCKED;

bool mutexes_init(void){
    serialMutex = xSemaphoreCreateMutex();
    if (serialMutex == NULL) {
        Serial.println("Failed to create serial mutex!");
        return false;
        while (1); // halt or retry
    }
    // wdtMutex = xSemaphoreCreateMutex();
    // if (wdtMutex == NULL) {
    //     Serial.println("Failed to create wdt mutex!");
    //     return false;
    //     while (1); // halt or retry
    // }

    return true;
}