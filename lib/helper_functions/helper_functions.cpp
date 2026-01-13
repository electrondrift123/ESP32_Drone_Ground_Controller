#include "helper_functions.h"

float float_constraint(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

float getBiasedValue(float raw, float center, float deadzone, float max_value) {
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
  float biased = pow(abs(normalized), 3.0); 
  if (normalized < 0) biased *= -1;

  return biased * 100; // Scale to 0-100 range
}