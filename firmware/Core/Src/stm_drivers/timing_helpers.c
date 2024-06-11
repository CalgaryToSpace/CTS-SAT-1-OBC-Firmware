#include "stm_drivers/timing_helpers.h"

/// @brief Delay blocking, not yielding to other RTOS tasks.
/// @param delay_time_ms Duration to delay, in milliseconds.
void TIM_delay_blocking_ms(uint32_t delay_time_ms) {
	HAL_Delay(delay_time_ms);
}

/// @brief Get the uptime of the system in milliseconds.
/// @return The number of milliseconds since the system started/booted.
uint32_t TIM_get_uptime_ms() {
	return HAL_GetTick();
}
