#ifndef __INCLUDE_GUARD__NUMBER_COMPARISONS_H__
#define __INCLUDE_GUARD__NUMBER_COMPARISONS_H__

#include <stdint.h>

/*
compare_doubles
REQUIRES:
    - a, b, epsilon are all doubles
PROMISES:
    - returns 1 if a and b are approximately equal to each other (within epsilon)
    - returns 0 otherwise
*/
uint8_t compare_doubles(double a, double b, double epsilon);

/*
compare_floats
REQUIRES:
    - a, b, epsilon are all floats
PROMISES:
    - returns 1 if a and b are approximately equal to each other (within epsilon)
    - returns 0 otherwise
*/
uint8_t compare_floats(float a, float b, float epsilon);

#endif // __INCLUDE_GUARD__NUMBER_COMPARISONS_H__
