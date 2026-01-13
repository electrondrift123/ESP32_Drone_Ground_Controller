#include <Arduino.h>
#include "pin_config.h"
#include "shared_data.h"
#include "sync.h"

void IRAM_ATTR handleKillButton(){
  kill_motor_state = !kill_motor_state;
}
void IRAM_ATTR handleELandButton(){
  e_land_state = !e_land_state;
}
void IRAM_ATTR handleAltHoldButton(){
  alt_hold_state = !alt_hold_state;
}
void IRAM_ATTR handlePIDModeButton(){
  pid_mode_state = !pid_mode_state;
}
void IRAM_ATTR handleLeftSwitch(){
  left_switch_state = !left_switch_state;
}
void IRAM_ATTR handleRightSwitch(){
  right_switch_state = !right_switch_state;
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

  pinMode(KILL_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(KILL_PIN), handleKillButton, RISING);
  pinMode(E_LAND_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(E_LAND_PIN), handleELandButton, RISING);
  pinMode(ALT_HOLD_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ALT_HOLD_PIN), handleAltHoldButton, RISING);
  pinMode(PID_MODE_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PID_MODE_PIN), handlePIDModeButton, RISING);
  pinMode(SW_L_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(SW_L_PIN), handleLeftSwitch, RISING);
  pinMode(SW_R_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(SW_R_PIN), handleRightSwitch, RISING);

  // pinMode(READ_ADC, INPUT);
  pinMode(T_PIN, INPUT);
  pinMode(Y_PIN, INPUT);
  pinMode(P_PIN, INPUT);
  pinMode(R_PIN, INPUT);
  analogReadResolution(12); // Set ADC resolution to 12 bits -> 4095 max

  // Take 40–60 samples and average them (reduces noise a lot)
  const int SAMPLES = 50;
  long sum_r = 0, sum_p = 0, sum_y = 0, sum_t = 0;

  for (int i = 0; i < SAMPLES; i++) {
    sum_r += analogRead(R_PIN);
    sum_p += analogRead(P_PIN);
    sum_y += analogRead(Y_PIN);
    sum_t += analogRead(T_PIN);
    delay(8);               // ~8 ms → total ~400 ms calibration
  }

  center_roll     = sum_r / (float)SAMPLES;
  center_pitch    = sum_p / (float)SAMPLES;
  center_yaw      = sum_y / (float)SAMPLES;
  center_throttle = sum_t / (float)SAMPLES;

  Serial.print("Calibrated centers →  roll: ");   Serial.print(center_roll, 1);
  Serial.print("  pitch: ");   Serial.print(center_pitch, 1);
  Serial.print("  yaw: ");     Serial.print(center_yaw, 1);
  Serial.print("  throttle: "); Serial.println(center_throttle, 1);
  Serial.println("Starting normal operation...\n");
}

void pin_config_init(void){
    led_pin_init();
}