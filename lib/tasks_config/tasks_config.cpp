#include "tasks_config.h"
#include "pin_config.h"
#include "main_tx.h"

void blinkTask(void* Parameters);
void txTask(void* Parameters);

BaseType_t taskResult;
bool freeRTOS_tasks_init(void){
    taskResult = xTaskCreatePinnedToCore(
        blinkTask,
        "blinking task",
        128,
        NULL,
        1,
        NULL,
        1
    );
    if (taskResult != pdPASS){
        Serial.println("blink task failed to create");
        return false;
    }

    taskResult = xTaskCreatePinnedToCore(
        txTask,
        "TxTask",
        4096,
        NULL,
        2,
        NULL,
        1
    );
    if (taskResult != pdPASS){
        Serial.println("TX task failed to create");
        return false;
    }

    return true;
}


void blinkTask(void* Parameters){

    TickType_t interval = pdMS_TO_TICKS(500);
    TickType_t lastWakeTime = xTaskGetTickCount();

    for (;;){
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        vTaskDelayUntil(&lastWakeTime, interval);
    }
}

void txTask(void* pvParams) {
  const TickType_t sendInterval = pdMS_TO_TICKS(250); // Send every 20ms (~50Hz)
  char payload[] = "FWD";  // Example payload, max 32 bytes

  while (1) {
    bool ok = radio.writeFast(&payload, strlen(payload));
    
    if (!ok) {
      Serial.println("[TX] FIFO full or failed to write");
      radio.flush_tx(); // Clear if overflowed
    }

    int16_t telemetry[5] = {0, 0, 0, 0, 0};

    if (radio.isAckPayloadAvailable()) {
      radio.read(&telemetry, sizeof(telemetry));
      Serial.printf("[ACK] roll: %d, pitch: %d, yaw: %d, alt: %d, Heading: %d\n",
                    telemetry[0], telemetry[1], telemetry[2],
                    telemetry[3], telemetry[4]);
    }

    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    vTaskDelay(sendInterval);
  }
}
