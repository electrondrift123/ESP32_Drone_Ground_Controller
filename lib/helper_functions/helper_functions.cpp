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

float velocityZControl(float throttle_input){
  // stick_input: should be in range [-100, 100]

  float max_rate = 80.0f; // max of [-0.8, 0.8] m/s climb and descent rate
  float deadzone = 10.0f; // 10% throttle deadzone for rate control

  if (fabs(throttle_input) < deadzone) return 0;

  float vz_cmd = throttle_input * max_rate / 100.0f; // Scale input to max rate 
  
  return vz_cmd; // Return the desired vertical velocity command [-80,80]
}