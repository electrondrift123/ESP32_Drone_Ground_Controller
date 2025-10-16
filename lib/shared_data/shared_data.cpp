#include "shared_data.h"

volatile bool SAFE_WDT = true; // Global flag to indicate WDT safety
volatile bool kill_motor_state = false; // Global flag to indicate motor kill state