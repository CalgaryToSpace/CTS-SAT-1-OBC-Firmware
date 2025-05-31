
#ifndef INCLUDE_GUARD__STM32_TIMING_HELPERS_H
#define INCLUDE_GUARD__STM32_TIMING_HELPERS_H

#include <stdint.h>

void TIM_delay_blocking_ms(uint32_t delay_time_ms);

uint32_t TIM_get_uptime_ms();

#endif /* INCLUDE_GUARD__STM32_TIMING_HELPERS_H */
