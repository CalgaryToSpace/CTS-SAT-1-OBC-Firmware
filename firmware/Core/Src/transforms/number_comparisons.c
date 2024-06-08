#include "transforms/number_comparisons.h"
#include <math.h>

/*
compare_doubles
REQUIRES:
    - a, b, epsilon are all doubles
PROMISES:
    - returns 1 if a and b are approximately equal to each other (within epsilon)
    - returns 0 otherwise
*/
uint8_t compare_doubles(double a, double b, double epsilon)
{
    return fabs(a - b) < epsilon;
}

/*
compare_floats
REQUIRES:
    - a, b, epsilon are all floats
PROMISES:
    - returns 1 if a and b are approximately equal to each other (within epsilon)
    - returns 0 otherwise
*/
uint8_t compare_floats(float a, float b, float epsilon)
{
    return fabs(a - b) < epsilon;
}
