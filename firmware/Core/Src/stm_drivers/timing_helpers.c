#include "stm_drivers/timing_helpers.h"

void delay_ms(uint32_t delay_time_ms) {
	HAL_Delay(delay_time_ms);
}

uint32_t get_uptime_ms() {
	return HAL_GetTick();
}
