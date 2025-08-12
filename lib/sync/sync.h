#ifndef SYNC_H
#define SYNC_H

#include <Arduino.h>

extern SemaphoreHandle_t serialMutex;

bool mutexes_init(void);

#endif