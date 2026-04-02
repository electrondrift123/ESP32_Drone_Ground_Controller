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

void read_analog(void* parameters){
  const TickType_t readInterval = pdMS_TO_TICKS(4); // 250 Hz
  TickType_t lastWakeTime = xTaskGetTickCount();

  // float local_T = 0.0f; // Example value, adjust as needed
  float local_T = 0.0f; 
  float local_Y = 0.0f; 
  float local_P = 0.0f;
  float local_R = 0.0f;

  float dt = 0.004f; // 4 ms in seconds
  static float current_throttle = 0.0f;

  for (;;){
    // local_T = ((analogRead(READ_ADC))); // [0, 1]
    local_T = safeAnalogRead(T_PIN); // Read throttle
    local_Y = safeAnalogRead(Y_PIN); // Read Y-axis
    local_P = safeAnalogRead(P_PIN); // Read P-axis
    local_R = safeAnalogRead(R_PIN); // Read R-axis

    // ADC read 12 bit res: getBiasedValue output: [-100,100]
    local_T = getBiasedValue(local_T, center_throttle, DEADZONE, MAX, 1.20f);
    local_Y = getBiasedValue(local_Y, center_yaw, DEADZONE, MAX, 2.5f) * (-1.0f); // Invert Yaw-axis for correct direction
    local_P = getBiasedValue(local_P, center_pitch, DEADZONE, MAX, 2.0f) * (-1.0f); // Invert Pitch-axis for correct direction
    local_R = getBiasedValue(local_R, center_roll, DEADZONE, MAX, 2.0f);

    // convert it to cmd angles (0-30 deg)
    local_R = local_R * (3000.0f / 100.0f);
    local_P = local_P * (3000.0f / 100.0f);
    local_Y = local_Y * 0.90f; // need also to be scaled 

  /////////////// put the throttle mech here ///////////////
  float T_max = 1000.0f; // Max throttle value (scaled to 0-1000)
  float rate = throttleRateControl(local_T / 100.0f); // [-1,1] Get rate control output based on throttle input
  current_throttle += rate * dt;

  // Clamp
  if (current_throttle < 0) current_throttle = 0;
  if (current_throttle > T_max) current_throttle = T_max;

  local_T = current_throttle;  // update the throttle value to be sent
  /////////////////////////////////////////////////////////

    // update the shared data
    if (xSemaphoreTake(loadMutex, pdMS_TO_TICKS(1)) == pdTRUE) {
      // scale by RX requirements: (100 for P,R,andY), (10 for T)
      load_data[0] = (int16_t)(local_T); // (0-1000) 
      load_data[1] = (int16_t)(local_Y * 100.0f); // Yaw: [-120.00, 120.00]
      load_data[2] = (int16_t)(local_P); // Pitch: [-30.00, 30.00]
      load_data[3] = (int16_t)(local_R); // Roll: [-30.00, 30.00]

      load_data[4] = kill_motor_state ? 1 : 0; // Convert to int for kill state
      xSemaphoreGive(loadMutex);
    }
    // update the kill switch led indicator
    // digitalWrite(KILL_LED_PIN, kill_motor_state);

    // // printing for displaying gains
    // if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(1)) == pdTRUE) {
    //   // Serial.print("T: "); Serial.println(local_T * (1000.0f/4095.0f)); // Print normalized value
    //   // Serial.print("R: "); Serial.println(local_R); // Print normalized value
    //   Serial.print("kill ,           "); Serial.println(kill_motor_state);
    //   Serial.print("eland ,          "); Serial.println(e_land_state);
    //   Serial.print("alt hold ,       "); Serial.println(alt_hold_state);
    //   Serial.print("pid ,            "); Serial.println(pid_mode_state);
    //   // Serial.print(" , "); Serial.println(left_switch_state);
    //   // Serial.print(" , "); Serial.println(right_switch_state);
    //   xSemaphoreGive(serialMutex);
    // }
    if (kill_motor_state){
      digitalWrite(KILL_LED_PIN, HIGH);
    } else {
      digitalWrite(KILL_LED_PIN, LOW);
    }

    vTaskDelayUntil(&lastWakeTime, readInterval);
  }
}

void txTask(void* pvParams) {
  // period that works: 100ms, 50ms, 20ms, 10ms, 5ms, _
  const TickType_t sendInterval = pdMS_TO_TICKS(4); // Send every 4ms (250 Hz) - to be tested
  TickType_t lastWakeTime = xTaskGetTickCount();

  vTaskDelay(pdMS_TO_TICKS(100));
  Serial.println("TX task start!");

  bool connected = false;
  // mode, kp, ki, kd, kill (scaled by 100)
  // T, Y, P, R, Kill (1 = kill, 0 = not), Eland, sigma, gamma
  float sigma = 0.01f;
  float gamma = 100.0f;
  int16_t sigma_ = (int16_t)(sigma * 1000.0f);
  // int16_t gamma_ = (int16_t)(gamma / 100.0f);
  int16_t gamma_ = (int16_t)(gamma);
  int16_t load_local[8] = {0, 0, 0, 0, 1, 0, sigma_, gamma_}; // +- 300.00 max res, x100

  uint16_t counter = 0;

  radio.flush_rx();
  radio.flush_tx();

  while (1) {
    // accessing the load data array
    if (xSemaphoreTake(loadMutex, pdMS_TO_TICKS(1)) == pdTRUE) {
      load_local[0] = load_data[0]; // Trottle 
      load_local[1] = load_data[1]; // Yaw
      load_local[2] = load_data[2]; // Pitch
      load_local[3] = load_data[3]; // Roll
      load_local[4] = load_data[4]; // Kill switch state
      load_local[5] = load_data[5]; // elanding
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
      int16_t telemetry[9]; // R,P,Y,alt,connection, P,kp,ki,kd
      radio.read(&telemetry, sizeof(telemetry));

      // if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(1)) == pdTRUE) {
      //   Serial.printf("[ACK] roll: %d, pitch: %d, yaw: %d, alt: %d, Connection: %d\n",
      //               telemetry[0], telemetry[1], telemetry[2],
      //               telemetry[3], telemetry[4]);
      //   Serial.println(load_local[0]);
      //   xSemaphoreGive(serialMutex);
      // }

      // read data sent here
      if (xSemaphoreTake(telemetryMutex, pdMS_TO_TICKS(1)) == pdTRUE){
        // update global telemetry
        for (int i = 0; i <= 8; i++){
          telemetry_data[i] = (float)telemetry[i];
        }
        xSemaphoreGive(telemetryMutex);
      }

      connected = (bool) telemetry[4];
    }
    // else connected = false;

    counter++;

    if (counter >= 10){ // every 0.5 seconds
      counter = 0;
      // WDT_setSafe(connected); // update wdt
      digitalWrite(RADIO_LED_PIN, LOW); // Update radio connection LED
      WDT_setSafe(true); // update wdt
    }

    digitalWrite(RADIO_LED_PIN, connected);

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
  float P, Kp, Ki, Kd;

  for (;;){
    // read mode (right SW)
    local_mode = right_switch_state ? 1 : 0; // 1: telemetry, 0: cmd

    // read telemetry data
    if (local_mode){ // display telemetry
      if (xSemaphoreTake(telemetryMutex, pdMS_TO_TICKS(1)) == pdTRUE) {
        roll = telemetry_data[0] / 100.0f;
        pitch = telemetry_data[1] / 100.0f;
        heading = telemetry_data[2] / 100.0f;
        alt = telemetry_data[3] / 100.0f;
        connection = telemetry_data[4];

        // temporary for analysis (roll only):
        // P = telemetry_data[5] / 100.0f;
        P = (float)telemetry_data[5];
        Kp = telemetry_data[6] / 100.0f;
        Ki = telemetry_data[7] / 100.0f;
        Kd = telemetry_data[8] / 100.0f;
        xSemaphoreGive(telemetryMutex);
      }

      if (connection > 0.0f){
        oled_displayTelemetry(roll, pitch, heading, P);
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

      // scale it back (for displaying purpose)
      local_T = local_T / 10.0f; // (0-100) %
      local_Y = local_Y / 100.0f; // Yaw: [-120.00, 120.00]
      local_P = local_P / 100.0f; // Pitch: [-30.00, 30.00]
      local_R = local_R / 100.0f; // Roll: [-30.00, 30.00]

      oled_displayCmd(local_T, local_Y, local_P, local_R);
    }

    vTaskDelayUntil(&lastWakeTime, update_interval);
  }
}

// Problems to be solved:
// 1. WDT keep resetting (temporarily fixed by always setting safe in txTask)


// toDO:
// - program the led combination signals for pid mode, e land, and alt hold!
