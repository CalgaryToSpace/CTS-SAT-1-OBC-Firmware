
#ifndef INCLUDE_GUARD__TIMING_HELPERS_H_
#define INCLUDE_GUARD__TIMING_HELPERS_H_


#include "main.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

void TIM_delay_blocking_ms(uint32_t delay_time_ms);

uint32_t TIM_get_uptime_ms();

#endif /* INCLUDE_GUARD__TIMING_HELPERS_H_ */
