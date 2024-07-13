#include "transforms/number_comparisons.h"
#include <math.h>

/// @brief Returns 1 if a and b are approximately equal to each other (within epsilon), 0 otherwise
/// @return Returns 1 if a and b are approximately equal to each other (within epsilon), 0 otherwise
uint8_t GEN_compare_doubles(double a, double b, double epsilon)
{
    return fabs(a - b) < epsilon;
}

/// @brief Returns 1 if a and b are approximately equal to each other (within epsilon), 0 otherwise
/// @return Returns 1 if a and b are approximately equal to each other (within epsilon), 0 otherwise
uint8_t GEN_compare_floats(float a, float b, float epsilon)
{
    return fabs(a - b) < epsilon;
}
