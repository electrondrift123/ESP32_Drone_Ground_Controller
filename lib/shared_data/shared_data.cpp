#include "shared_data.h"

volatile bool SAFE_WDT = true; // Global flag to indicate WDT safety
volatile bool kill_motor_state = true; // Global flag to indicate motor kill state
volatile bool e_land_state = false; // Global flag for emergency landing state
volatile bool alt_hold_state = false; // Global flag for altitude hold state
volatile bool pid_mode_state = false; // Global flag for PID mode state
volatile bool left_switch_state = false; // Global flag for left switch state
volatile bool right_switch_state = false; // Global flag for right switch state

// initial joystick values (true center)
float center_roll = 0.0f;
float center_pitch = 0.0f;
float center_yaw = 0.0f;
float center_throttle = 0.0f;

int16_t load_data[8] = {0, 0, 0, 0, 1, 0, 0, 0}; // Load data array initialized to zero
float telemetry_data[9] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}; // Telemetry data array initialized to zero
