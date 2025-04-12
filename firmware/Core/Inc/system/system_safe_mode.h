#ifndef INCLUDE_GUARD__SYS_SAFE_MODE
#define INCLUDE_GUARD__SYS_SAFE_MODE

#include <stdint.h>

uint8_t SYS_enter_safe_mode();
uint8_t SYS_eps_status_safe_mode_check();
uint8_t SYS_battery_safe_mode_check();

#endif // INCLUDE_GUARD__SYS_SAFE_MODE