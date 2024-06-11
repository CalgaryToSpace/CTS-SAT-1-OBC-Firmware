
#ifndef __INCLUDE_GUARD__TIMING_HELPERS_H_
#define __INCLUDE_GUARD__TIMING_HELPERS_H_


#include "main.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

void delay_ms(uint32_t delay_time_ms);

uint32_t get_uptime_ms();

#endif /* __INCLUDE_GUARD__TIMING_HELPERS_H_ */
