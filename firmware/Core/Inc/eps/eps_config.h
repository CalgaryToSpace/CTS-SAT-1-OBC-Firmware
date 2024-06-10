#ifndef __EPS_CONFIG_H
#define __EPS_CONFIG_H

#include "../configuration/configuration.h"

extern uint8_t test_config_eps = 1;

extern const Configuration_Variable EPS_CONFIG[] = {
    {.var = &test_config_eps,
     .subsystem = CTS_Subsystem_EPS}};

#endif