#include <Arduino.h>
#include "pin_config.h"

void led_pin_init(void){
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
}

void pin_config_init(void){
    led_pin_init();
}