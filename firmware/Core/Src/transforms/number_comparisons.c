#include "transforms/number_comparisons.h"

/// @brief Returns the absolute value of a.
/// @param a 
/// @return The absolute value of `a`.
/// @note This is a reimplementation of `fabs` in C, because Cryptolib creates a file named math.h.
double GEN_double_abs(double a) {
    if (a < 0) {
        return -a;
    }
    return a;
}


/// @brief Returns 1 if a and b are approximately equal to each other (within epsilon), 0 otherwise
/// @return Returns 1 if a and b are approximately equal to each other (within epsilon), 0 otherwise
uint8_t GEN_compare_doubles(double a, double b, double epsilon)
{
    return GEN_double_abs(a - b) < epsilon;
}

/// @brief Returns 1 if a and b are approximately equal to each other (within epsilon), 0 otherwise
/// @return Returns 1 if a and b are approximately equal to each other (within epsilon), 0 otherwise
uint8_t GEN_compare_floats(float a, float b, float epsilon)
{
    return GEN_double_abs(a - b) < epsilon;
}
