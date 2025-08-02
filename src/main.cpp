#include <Arduino.h>
#include <SPI.h>
#include "pin_config.h"
#include "tasks_config.h"
#include "main_tx.h"

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  // delay(250);
  while(!Serial);
  Serial.println("Serial Ready!");

  SPI.begin();
  delay(250);
  Serial.println("SPI ready!");

  pin_config_init();
  Serial.println("pin config init success!");
  if (!main_tx_init()){
    Serial.println("Failed to initialize main TX task");
    // while(1); // halt or retry
  }
  Serial.println("main tx init success!");
  if (!freeRTOS_tasks_init()){
    Serial.println("Failed to create freeRTOS tasks");
    // while(1); // halt or retry
  }else {Serial.println("freeRTOS init success!");}

  Serial.println("freeRTOS tasks will execute now!");

  vTaskStartScheduler();
}

void loop() {
  // put your main code here, to run repeatedly:
}
