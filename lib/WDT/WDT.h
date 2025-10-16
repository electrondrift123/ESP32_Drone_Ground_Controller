#ifndef WDT_H
#define WDT_H

#include <Arduino.h>
#include "esp_task_wdt.h"  // WDT library for ESP32

#define WDT_TIMEOUT 2 // 2 seconds

void WDT_init(void);
void WDT_setSafe(bool state);
bool WDT_isSafe(void);

#endif