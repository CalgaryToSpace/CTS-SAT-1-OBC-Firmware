#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <stdint.h>

typedef enum
{
    CTS_Subsystem_EPS
} CTS_Subsystems;

typedef struct
{
    void *var;
    CTS_Subsystems subsystem;
} Configuration_Variable;

#endif