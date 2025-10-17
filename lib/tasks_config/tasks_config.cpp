#include "tasks_config.h"
#include "pin_config.h"
#include "main_tx.h"

#include "sync.h"

#include <Arduino.h>
#include <SPI.h>
#include "RF24.h"

#include "WDT.h"  // WDT library for ESP32
#include "shared_data.h"
#include "helper_functions.h"

void read_analog(void* Parameters);
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

void read_analog(void* parameters){
  const TickType_t readInterval = pdMS_TO_TICKS(100); // Read every 100ms
  TickType_t lastWakeTime = xTaskGetTickCount();

  float local_kp = 1; // Example value, adjust as needed

  for (;;){
    // read the P-gain (one at a time)
    if (xSemaphoreTake(adcMutex, portMAX_DELAY)) {
      local_kp = ((39.0 / 4095) * analogRead(READ_ADC)) + 1; // [1.00, 40.00]
      xSemaphoreGive(adcMutex);
    }
    // update the shared data
    if (xSemaphoreTake(loadMutex, portMAX_DELAY)) {
      load_data[1] = (int16_t)(local_kp * 100); // Scale to 2 decimal places
      load_data[4] = kill_motor_state ? 1 : 0; // Convert to int for kill state
      xSemaphoreGive(loadMutex);
    }
    // update the kill switch led indicator
    digitalWrite(KILL_LED_PIN, kill_motor_state);

    // // printing for displaying gains
    // if (xSemaphoreTake(serialMutex, portMAX_DELAY)){
    //   Serial.print("Kp: "); Serial.println(local_kp);
    //   xSemaphoreGive(serialMutex);
    // }

    vTaskDelayUntil(&lastWakeTime, readInterval);
  }
}

void txTask(void* pvParams) {
  const TickType_t sendInterval = pdMS_TO_TICKS(50); // Send every 20ms (~50Hz)
  TickType_t lastWakeTime = xTaskGetTickCount();

  vTaskDelay(pdMS_TO_TICKS(100));
  Serial.println("TX task start!");

  bool connected = 1;
  int16_t load_local[5] = {100, 100, 0, 0, 0}; // +- 300.00 max res, x100

  uint16_t counter = 0;

  radio.flush_rx();
  radio.flush_tx();

  while (1) {
    // accessing the load data array
    if (xSemaphoreTake(loadMutex, portMAX_DELAY)){
      load_local[1] = load_data[1];
      load_local[4] = load_data[4];
      xSemaphoreGive(loadMutex);
    }

    bool ok = radio.writeFast(load_local, sizeof(load_local));
    
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
      // WDT_setSafe(connected); // update wdt
      WDT_setSafe(true); // update wdt
    }

    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    vTaskDelayUntil(&lastWakeTime, sendInterval);
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

// Problems to be solved:
// 1. WDT keep resetting 
// 2. Radio Malfunction, do receive but 0 data. 
// radio.isAckPayloadAvailable() is returning true.
// its output: [ACK] roll: 0, pitch: 0, yaw: 0, alt: 0, Connection: 0