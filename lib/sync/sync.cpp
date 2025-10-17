#include <Arduino.h>
#include "sync.h"

SemaphoreHandle_t serialMutex;
// SemaphoreHandle_t wdtMutex;
SemaphoreHandle_t adcMutex = NULL; // Initialize to NULL

SemaphoreHandle_t loadMutex = NULL;
SemaphoreHandle_t telemetryMutex = NULL;

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
    adcMutex = xSemaphoreCreateMutex();
    if (adcMutex == NULL) {
        Serial.println("Failed to create ADC mutex!");
        return false;
        while (1); // halt or retry
    }
    loadMutex = xSemaphoreCreateMutex();
    if (loadMutex == NULL) {
        Serial.println("Failed to create load mutex!");
        return false;
        while (1); // halt or retry
    }
    telemetryMutex = xSemaphoreCreateMutex();
    if (telemetryMutex == NULL) {
        Serial.println("Failed to create telemetry mutex!");
        return false;
        while (1); // halt or retry
    }

    return true;
}