#ifndef SHARED_DATA_H
#define SHARED_DATA_H

#include <Arduino.h>

extern volatile bool SAFE_WDT;
extern volatile bool kill_motor_state;

extern int16_t load_data[5]; // Load data array
extern int16_t telemetry_data[5]; // received telemetry data array

#endif