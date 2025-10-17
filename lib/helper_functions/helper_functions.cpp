#include "helper_functions.h"

float float_constraint(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}