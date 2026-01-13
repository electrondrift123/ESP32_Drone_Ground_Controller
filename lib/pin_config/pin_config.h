#ifndef PIN_CONFIG_H
#define PIN_CONFIG_H

#include <Arduino.h>

#define LED_PIN 2

// Joysticks
#define T_PIN 34 // Y
#define Y_PIN 35 // X
#define P_PIN 33 // Y
#define R_PIN 32 // X

#define SW_L_PIN 27
#define SW_R_PIN 25
#define KILL_PIN 14
#define E_LAND_PIN 12
#define ALT_HOLD_PIN 13
#define PID_MODE_PIN 17

// define PINs for ADC
// #define READ_ADC 33
// #define SW_PIN 32
#define KILL_LED_PIN 4
// for Flags

// VSPI
// #define SCK_PIN 18
// #define MISO_PIN 19
// #define MOSI_PIN 23

// nRF24
#define NRF_CE_PIN    26
#define NRF_CSN_PIN   5

void pin_config_init(void);
float safeAnalogRead(int pin);
void IRAM_ATTR handleKillButton(void);
void IRAM_ATTR handleELandButton(void);
void IRAM_ATTR handleAltHoldButton(void);
void IRAM_ATTR handlePIDModeButton(void);
void IRAM_ATTR handleLeftSwitch(void);
void IRAM_ATTR handleRightSwitch(void);

#endif