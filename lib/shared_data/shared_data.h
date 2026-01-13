#ifndef SHARED_DATA_H
#define SHARED_DATA_H

#include <Arduino.h>

// initial joystick values (true center)
extern float center_roll, center_pitch, center_yaw, center_throttle;
#define DEADZONE 10.0f
#define MAX 4095.0f

extern volatile bool SAFE_WDT;
extern volatile bool kill_motor_state;
extern volatile bool e_land_state;
extern volatile bool alt_hold_state;
extern volatile bool pid_mode_state;
extern volatile bool left_switch_state;
extern volatile bool right_switch_state;

extern int16_t load_data[5]; // Load data array
extern int16_t telemetry_data[5]; // received telemetry data array

#endif