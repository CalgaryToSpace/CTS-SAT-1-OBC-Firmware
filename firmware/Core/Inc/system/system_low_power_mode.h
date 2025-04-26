#ifndef INCLUDE_GUARD__SYS_LOW_POWER_MODE
#define INCLUDE_GUARD__SYS_LOW_POWER_MODE

#include <stdint.h>

// Number of channels that are disabled when entering low power mode
#define LOW_POWER_MODE_DISABLED_CHANNEL_COUNT 4
#define LOW_POWER_MODE_JSON_STRING_LEN 275

// Order matters here as all channels are disabled first
typedef enum {
    SYS_LOW_POWER_MODE_ERROR_MPI_5V = 1 << 0,
    SYS_LOW_POWER_MODE_ERROR_MPI_12V = 1 << 1,
    SYS_LOW_POWER_MODE_ERROR_CAMERA = 1 << 2,
    SYS_LOW_POWER_MODE_ERROR_BOOM_12V = 1 << 3,
    SYS_LOW_POWER_MODE_ERROR_BOOM_PINS = 1 << 4,
    SYS_LOW_POWER_MODE_ERROR_GPS = 1 << 5,
    SYS_LOW_POWER_MODE_ERROR_ADCS = 1 << 6,
} SYS_low_power_mode_error_enum_t;


uint8_t SYS_enter_low_power_mode();
uint8_t SYS_check_eps_and_enter_low_power_mode();
uint8_t SYS_check_battery_and_enter_low_power_mode();
char* SYS_low_power_mode_error_enum_to_string(SYS_low_power_mode_error_enum_t error_mask);
uint8_t SYS_low_power_mode_error_result_to_json(SYS_low_power_mode_error_enum_t error, char *buffer, uint16_t buffer_size);

#endif // INCLUDE_GUARD__SYS_LOW_POWER_MODE