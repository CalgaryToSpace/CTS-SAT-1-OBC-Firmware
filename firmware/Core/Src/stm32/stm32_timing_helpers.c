#include "stm32/stm32_timing_helpers.h"

#include "main.h"

/// @brief Delay blocking, not yielding to other RTOS tasks.
/// @param delay_time_ms Duration to delay, in milliseconds.
void TIM_delay_blocking_ms(uint32_t delay_time_ms) {
	HAL_Delay(delay_time_ms);
}
