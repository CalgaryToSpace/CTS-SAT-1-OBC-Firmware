#include "transforms/number_comparisons.h"
#include <math.h>
#include <stdint.h>

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

/// @brief Returns the absolute value of an int64_t
/// @param value Input signed 64-bit number
/// @return Unsigned 64-bit number which is the absolute value of the input
uint64_t GEN_abs_int64(int64_t value) {
    if (value < 0) {
        return -value;
    }
    return value;
}
