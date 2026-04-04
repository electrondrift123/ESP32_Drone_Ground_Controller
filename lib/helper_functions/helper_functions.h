#ifndef HELPER_FUNCTIONS_H
#define HELPER_FUNCTIONS_H

#include <Arduino.h>

#define CENTER 4095.0f / 2.0f // Center value for the joystick (assuming 12-bit ADC)

float float_constraint(float value, float min, float max);

float getBiasedValue(float raw, float center, float deadzone, float max_value, float power);

float velocityZControl(float throttle_input);

#endif // HELPER_FUNCTIONS_H