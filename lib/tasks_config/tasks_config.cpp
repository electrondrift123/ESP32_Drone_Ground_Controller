#include "tasks_config.h"
#include "pin_config.h"
#include "main_tx.h"

#include "sync.h"

#include <Arduino.h>
#include <SPI.h>
#include "RF24.h"

#include "WDT.h"  // WDT library for ESP32
#include "shared_data.h"

void txTask(void* Parameters);
void wdtTask(void* Parameters);

BaseType_t taskResult;
bool freeRTOS_tasks_init(void){
  taskResult = xTaskCreatePinnedToCore(
    txTask,
    "TxTask",
    4096,
    NULL,
    1,
    NULL,
    1
  );
  if (taskResult != pdPASS){
    Serial.println("TX task failed to create");
    return false;
  }

  taskResult = xTaskCreatePinnedToCore(
    wdtTask,
    "WDTTask",
    2048,
    NULL,
    1,
    NULL,
    0
  );
  if (taskResult != pdPASS){
    Serial.println("WDT task failed to create");
    return false;
  }

  return true;
}

void txTask(void* pvParams) {
  const TickType_t sendInterval = pdMS_TO_TICKS(50); // Send every 20ms (~50Hz)

  vTaskDelay(pdMS_TO_TICKS(100));
  Serial.println("TX task start!");

  bool connected = 1;
  int16_t load[5] = {1, 1, 0, 0, 0}; // +- 300.00 max

  uint16_t counter = 0;

  while (1) {
    bool ok = radio.writeFast(load, sizeof(load));
    
    if (!ok) {
      Serial.println("[TX] FIFO full or failed to write");
      radio.flush_tx(); // Clear if overflowed
    }else {
      radio.txStandBy();  // this is the fix that ensures TX completes
    }

    int16_t telemetry[5];

    if (radio.isAckPayloadAvailable()) {
      radio.read(&telemetry, sizeof(telemetry));
      if (xSemaphoreTake(serialMutex, portMAX_DELAY)) {
        Serial.printf("[ACK] roll: %d, pitch: %d, yaw: %d, alt: %d, Connection: %d\n",
                    telemetry[0], telemetry[1], telemetry[2],
                    telemetry[3], telemetry[4]);
        xSemaphoreGive(serialMutex);
      }

      connected = (bool) telemetry[4];
    }else connected = false;

    counter++;

    if (counter >= 10){ // every 0.5 seconds
      counter = 0;
      WDT_setSafe(connected); // update wdt
    }

    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    vTaskDelay(sendInterval);
  }
}

void wdtTask(void* Parameters){
  const TickType_t refresh_interval = pdMS_TO_TICKS(500); 

  // WDT init
  WDT_init();
  Serial.println("Watchdog initialized!");

  for (;;){
    if (WDT_isSafe()){
      esp_task_wdt_reset(); // refresh
    }
    vTaskDelay(refresh_interval);
  }
}

