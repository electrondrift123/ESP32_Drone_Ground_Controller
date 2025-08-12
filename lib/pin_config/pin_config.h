#ifndef PIN_CONFIG_H
#define PIN_CONFIG_H

#include <Arduino.h>

#define LED_PIN 2

// define PINs for ADC

// for Flags

// VSPI
// #define SCK_PIN 18
// #define MISO_PIN 19
// #define MOSI_PIN 23

// nRF24
#define NRF_CE_PIN    26
#define NRF_CSN_PIN   5

void pin_config_init(void);


#endif