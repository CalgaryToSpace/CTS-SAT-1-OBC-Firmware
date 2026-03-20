#ifndef INCLUDE_GUARD__NUMBER_COMPARISONS_H__
#define INCLUDE_GUARD__NUMBER_COMPARISONS_H__

#include <stdint.h>

uint8_t GEN_compare_doubles(double a, double b, double epsilon);

uint8_t GEN_compare_floats(float a, float b, float epsilon);

uint64_t GEN_abs_int64(int64_t value);

#endif // INCLUDE_GUARD__NUMBER_COMPARISONS_H__
