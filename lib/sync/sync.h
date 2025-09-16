#ifndef SYNC_H
#define SYNC_H

#include <Arduino.h>

extern SemaphoreHandle_t serialMutex;
// extern SemaphoreHandle_t wdtMutex;

// Define and initialize the spinlock
extern portMUX_TYPE wdtMutex;

bool mutexes_init(void);

#endif