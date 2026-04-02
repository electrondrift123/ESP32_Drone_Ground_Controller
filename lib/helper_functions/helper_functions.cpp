#include "helper_functions.h"

float float_constraint(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

float getBiasedValue(float raw, float center, float deadzone, float max_value, float power) {
  // 1. Check if inside deadzone
  if (abs(raw - center) < deadzone) return 0.0f;

  float normalized;
  // 2. Map linearly based on which side of center we are on
  if (raw < center) {
    normalized = (float)(raw - (center - deadzone)) / (center - deadzone); 
    // result is -1.0 to 0.0
  } else {
    normalized = (float)(raw - (center + deadzone)) / (max_value - (center + deadzone));
    // result is 0.0 to 1.0
  }

  // 3. Apply your "Heavy Bias" (Exponential Scaling)
  float biased = pow(abs(normalized), power); 
  if (normalized < 0) biased *= -1;

  return biased * 100; // Scale to 0-100 range
}

float throttleRateControl(float throttle_input){
  // stick_input: should be in range [-1, 1]
  // Returns: throttle rate change (% per second, but scaled to your 0-1000 range)

  float max_rate = 120.0f; // 120% throttle / second
  float deadzone = 0.10f; // 10% throttle deadzone for rate control

  if (fabs(throttle_input) < deadzone) return 0;

  float rate_percent = throttle_input * max_rate; // Scale input to max rate

  // Convert to your scale: 100% = 1000 units
  // So 120%/sec = 1200 units/sec
  float rate_units = rate_percent * 10.0f;  // because 100% = 1000 units
  
  return rate_units;
}