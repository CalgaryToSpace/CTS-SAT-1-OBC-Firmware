#ifndef INCLUDE_GUARD__BOOM_DEPLOY_DRIVERS_H
#define INCLUDE_GUARD__BOOM_DEPLOY_DRIVERS_H


#include "main.h"
#include <stdint.h>


uint8_t BOOM_get_pgood_status();

void BOOM_set_burn_enabled(uint8_t enabled);

#endif
