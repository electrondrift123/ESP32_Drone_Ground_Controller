#include <Arduino.h>
#include "pin_config.h"
#include "shared_data.h"
#include "sync.h"

void IRAM_ATTR handleKillButton(){
  kill_motor_state = !kill_motor_state;
}

float safeAnalogRead(int pin) {
  if (adcMutex != NULL) xSemaphoreTake(adcMutex, portMAX_DELAY);
  vTaskDelay(pdMS_TO_TICKS(1)); // Stabilize ADC
  float value = analogRead(pin);
  if (adcMutex != NULL) xSemaphoreGive(adcMutex);
  return value;
}

void led_pin_init(void){
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    pinMode(KILL_LED_PIN, OUTPUT);
    digitalWrite(KILL_LED_PIN, LOW);

    pinMode(SW_PIN, INPUT_PULLDOWN);
    attachInterrupt(digitalPinToInterrupt(SW_PIN), handleKillButton, RISING);

    pinMode(READ_ADC, INPUT);
    analogReadResolution(12); // Set ADC resolution to 12 bits
}

void pin_config_init(void){
    led_pin_init();
}