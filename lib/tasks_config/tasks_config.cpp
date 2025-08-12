#include "tasks_config.h"
#include "pin_config.h"
#include "main_tx.h"

#include "sync.h"

#include <Arduino.h>
#include <SPI.h>
#include "RF24.h"

void txTask(void* Parameters);

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

  return true;
}

void txTask(void* pvParams) {
  const TickType_t sendInterval = pdMS_TO_TICKS(50); // Send every 20ms (~50Hz)
  char payload[] = "yo";  // Example payload, max 32 bytes

  vTaskDelay(pdMS_TO_TICKS(1000));
  Serial.println("TX task start!");

  bool connected = 1;

  while (1) {
    bool ok = radio.writeFast(&payload, strlen(payload));
    
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

      // connected = (bool) telemetry[4];
    }

    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    vTaskDelay(sendInterval);
  }
}
