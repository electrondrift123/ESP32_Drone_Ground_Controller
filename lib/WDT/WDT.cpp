#include "WDT.h"
#include "shared_data.h"
#include "esp_task_wdt.h"  // WDT library for ESP32
#include "sync.h"

// void WDT_init(void){
//     // Create configuration struct
//     esp_task_wdt_config_t wdt_config = {
//         .timeout_ms = WDT_TIMEOUT * 1000, // milliseconds
//         .idle_core_mask = (1 << portNUM_PROCESSORS) - 1, // Watch all cores
//         .trigger_panic = true // Reset on timeout
//     };
//     // Initialize watchdog with config
//     esp_task_wdt_init(&wdt_config);
//     esp_task_wdt_add(NULL); // Add loop task to WDT
// }

void WDT_init(void) {
    // Initialize watchdog with timeout in seconds
    esp_task_wdt_init(WDT_TIMEOUT, true);  // true = trigger panic on timeout
    esp_task_wdt_add(NULL); // Add current task (loop task) to WDT
}


void WDT_setSafe(bool state) {
    taskENTER_CRITICAL(&wdtMutex);
    SAFE_WDT = state;
    taskEXIT_CRITICAL(&wdtMutex);
}

bool WDT_isSafe() {
    bool value;
    taskENTER_CRITICAL(&wdtMutex);
    value = SAFE_WDT;
    taskEXIT_CRITICAL(&wdtMutex);
    return value;
}