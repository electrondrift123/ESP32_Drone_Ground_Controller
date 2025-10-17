#include "shared_data.h"

volatile bool SAFE_WDT = true; // Global flag to indicate WDT safety
volatile bool kill_motor_state = false; // Global flag to indicate motor kill state

int16_t load_data[5] = {0, 0, 0, 0, 0}; // Load data array initialized to zero
int16_t telemetry_data[5] = {0, 0, 0, 0, 0}; // Telemetry data array initialized to zero