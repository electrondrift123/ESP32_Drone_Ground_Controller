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

#include "oled.h"

void read_analog(void* Parameters);
void txTask(void* Parameters);
void wdtTask(void* Parameters);
void oledTask(void* Parameters);

BaseType_t taskResult;
bool freeRTOS_tasks_init(void){
  taskResult = xTaskCreatePinnedToCore(
    read_analog,
    "Joystick Read Task",
    1024,
    NULL,
    1,
    NULL,
    0
  );
  if (taskResult != pdPASS){
    Serial.println("Analog read task failed to create");
    return false;
  }

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

  taskResult = xTaskCreatePinnedToCore(
    oledTask,
    "OLED Task",
    4096,
    NULL,
    1,
    NULL,
    0
  );
  if (taskResult != pdPASS){
    Serial.println("OLED task failed to create");
    return false;
  }

  return true;
}

// void read_analog(void* parameters){
//   const TickType_t readInterval = pdMS_TO_TICKS(4); // 250 Hz
//   TickType_t lastWakeTime = xTaskGetTickCount();

//   float local_T = 0.0f; // Example value, adjust as needed

//   for (;;){
//     // read the P-gain (one at a time)
//     if (xSemaphoreTake(adcMutex, pdMS_TO_TICKS(1)) == pdTRUE) {
//       local_T = ((analogRead(READ_ADC))); // [0, 1]
//       xSemaphoreGive(adcMutex);
//     }

//     // Potentio meter (temporary)
//     local_T = local_T * (700.0f / 4095.0f);
//     if (local_T > 700.0f) local_T = 700.0f; 
//     else if (local_T < 0.0f) local_T = 0.0f;

//     // update the shared data
//     if (xSemaphoreTake(loadMutex, pdMS_TO_TICKS(1)) == pdTRUE) {
//       // load_data[0] = (int16_t)(local_T * (100.0f / 4095.0f)); // Scale to 2 decimal places
//       load_data[0] = (int16_t)(local_T * (100.0f / 700.0f)); // Scale to 2 decimal places
//       load_data[4] = kill_motor_state ? 1 : 0; // Convert to int for kill state
//       xSemaphoreGive(loadMutex);
//     }
//     // update the kill switch led indicator
//     digitalWrite(KILL_LED_PIN, kill_motor_state);

//     // // printing for displaying gains
//     // if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(1)) == pdTRUE) {
//     //   // Serial.print("T: "); Serial.println(local_T * (1000.0f/4095.0f)); // Print normalized value
//     //   Serial.print("T: "); Serial.println(local_T); // Print normalized value
//     //   xSemaphoreGive(serialMutex);
//     // }

//     vTaskDelayUntil(&lastWakeTime, readInterval);
//   }
// }

void read_analog(void* parameters){
  const TickType_t readInterval = pdMS_TO_TICKS(4); // 250 Hz
  TickType_t lastWakeTime = xTaskGetTickCount();

  // float local_T = 0.0f; // Example value, adjust as needed
  float local_T = 0.0f; 
  float local_Y = 0.0f; 
  float local_P = 0.0f;
  float local_R = 0.0f;

  for (;;){
    // local_T = ((analogRead(READ_ADC))); // [0, 1]
    local_T = safeAnalogRead(T_PIN); // Read throttle
    local_Y = safeAnalogRead(Y_PIN); // Read Y-axis
    local_P = safeAnalogRead(P_PIN); // Read P-axis
    local_R = safeAnalogRead(R_PIN); // Read R-axis

    // ADC read 12 bit res
    local_T = getBiasedValue(local_T, center_throttle, DEADZONE, MAX);
    local_Y = getBiasedValue(local_Y, center_yaw, DEADZONE, MAX) * (-1.0f); // Invert Yaw-axis for correct direction
    local_P = getBiasedValue(local_P, center_pitch, DEADZONE, MAX) * (-1.0f); // Invert Pitch-axis for correct direction
    local_R = getBiasedValue(local_R, center_roll, DEADZONE, MAX);

    // convert it to cmd angles (0-30)
    local_R = local_R * (30.0f / 100.0f);
    local_P = local_P * (30.0f / 100.0f);
    local_Y = local_Y * 1.80f;

    if (local_T < 0) local_T = 0.0f;

    local_T = local_T * 70.0f / 100.0f; // 70% max throttle

    // update the shared data
    if (xSemaphoreTake(loadMutex, pdMS_TO_TICKS(1)) == pdTRUE) {
      // load_data[0] = (int16_t)(local_T * (100.0f / 4095.0f)); // Scale to 2 decimal places
      load_data[0] = (int16_t)local_T; 
      load_data[1] = (int16_t)local_Y; // Yaw
      load_data[2] = (int16_t)local_P; // Pitch
      load_data[3] = (int16_t)local_R; // Roll

      load_data[4] = kill_motor_state ? 1 : 0; // Convert to int for kill state
      xSemaphoreGive(loadMutex);
    }
    // update the kill switch led indicator
    digitalWrite(KILL_LED_PIN, kill_motor_state);

    // // printing for displaying gains
    // if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(1)) == pdTRUE) {
    //   // Serial.print("T: "); Serial.println(local_T * (1000.0f/4095.0f)); // Print normalized value
    //   // Serial.print("R: "); Serial.println(local_R); // Print normalized value
    //   // Serial.print(" , "); Serial.println(kill_motor_state);
    //   // Serial.print(" , "); Serial.println(e_land_state);
    //   // Serial.print(" , "); Serial.println(alt_hold_state);
    //   // Serial.print(" , "); Serial.println(pid_mode_state);
    //   // Serial.print(" , "); Serial.println(left_switch_state);
    //   // Serial.print(" , "); Serial.println(right_switch_state);
    //   xSemaphoreGive(serialMutex);
    // }

    vTaskDelayUntil(&lastWakeTime, readInterval);
  }
}

void txTask(void* pvParams) {
  // period that works: 100ms, 50ms, 20ms, 10ms, 5ms, _
  const TickType_t sendInterval = pdMS_TO_TICKS(5); // Send every 4ms (250 Hz) - to be tested
  TickType_t lastWakeTime = xTaskGetTickCount();

  vTaskDelay(pdMS_TO_TICKS(100));
  Serial.println("TX task start!");

  bool connected = false;
  // mode, kp, ki, kd, kill (scaled by 100)
  // T, Y, P, R, Kill (1 = kill, 0 = not)
  int16_t load_local[5] = {0, 0, 0, 0, 0}; // +- 300.00 max res, x100

  uint16_t counter = 0;

  radio.flush_rx();
  radio.flush_tx();

  while (1) {
    // accessing the load data array
    if (xSemaphoreTake(loadMutex, pdMS_TO_TICKS(1)) == pdTRUE) {
      load_local[0] = load_data[0]; // Trottle for calibration
      load_local[1] = load_data[1]; // Yaw
      load_local[2] = load_data[2]; // Pitch
      load_local[3] = load_data[3]; // Roll
      // load_local[4] = load_data[4]; // Kill switch state
      xSemaphoreGive(loadMutex);
    }

    bool ok = radio.writeFast(load_local, sizeof(load_local)); // works on 80ms, no txstandby
    // bool ok = radio.write(load_local, sizeof(load_local));
    
    if (!ok) {
      Serial.println("[TX] FIFO full or failed to write");
      radio.flush_tx(); // Clear if overflowed
    }
    else {
      bool sent = radio.txStandBy(100);  // this is the fix that ensures TX completes
      if (!sent) Serial.println("[TX] txStandBy TIMEOUT");
    }


    while (radio.isAckPayloadAvailable()) {
      int16_t telemetry[5];
      radio.read(&telemetry, sizeof(telemetry));

      if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(1)) == pdTRUE) {
        Serial.printf("[ACK] roll: %d, pitch: %d, yaw: %d, alt: %d, Connection: %d\n",
                    telemetry[0], telemetry[1], telemetry[2],
                    telemetry[3], telemetry[4]);
        Serial.println(load_local[0]);
        xSemaphoreGive(serialMutex);
      }

      connected = (bool) telemetry[4];
    }
    // else connected = false;

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

void oledTask(void* Parameters){
  const TickType_t update_interval = pdMS_TO_TICKS(100); 
  TickType_t lastWakeTime = xTaskGetTickCount();

  int local_mode = 0; // 0: cmd, 1: telemetry

  // cmd:
  float local_T = 0.0f;
  float local_Y = 0.0f;
  float local_P = 0.0f;
  float local_R = 0.0f;

  // telemetry:
  float roll = 0.0f;
  float pitch = 0.0f;
  float heading = 0.0f;
  float alt = 0.0f;
  float connection = 0.0f;

  for (;;){
    // read mode (right SW)
    local_mode = right_switch_state ? 1 : 0; // 1: telemetry, 0: cmd

    // read telemetry data
    if (local_mode){ // display telemetry
      if (xSemaphoreTake(telemetryMutex, pdMS_TO_TICKS(1)) == pdTRUE) {
        roll = (float)telemetry_data[0] / 100.0f;
        pitch = (float)telemetry_data[1] / 100.0f;
        heading = (float)telemetry_data[2] / 100.0f;
        alt = (float)telemetry_data[3] / 100.0f;

        xSemaphoreGive(telemetryMutex);
      }

      if (connection > 0.0f){
        oled_displayTelemetry(roll, pitch, heading, alt);
      } else {
        oled_displayNoConnection();
      }
    }

    else { // display cmd
      if (xSemaphoreTake(loadMutex, pdMS_TO_TICKS(1)) == pdTRUE) {
        local_T = (float)load_data[0];
        local_Y = (float)load_data[1];
        local_P = (float)load_data[2];
        local_R = (float)load_data[3];
        xSemaphoreGive(loadMutex);
      }

      oled_displayCmd(local_T, local_Y, local_P, local_R);
    }

    vTaskDelayUntil(&lastWakeTime, update_interval);
  }
}

// Problems to be solved:
// 1. WDT keep resetting (temporarily fixed by always setting safe in txTask)
