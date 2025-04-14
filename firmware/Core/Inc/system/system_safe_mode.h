#ifndef INCLUDE_GUARD__SYS_SAFE_MODE
#define INCLUDE_GUARD__SYS_SAFE_MODE

#include <stdint.h>
// Order matters here as all the systems disabled using channels are first
typedef enum {
    SYS_SAFE_MODE_ERROR_MPI_5V = 1 << 0,
    SYS_SAFE_MODE_ERROR_MPI_12V = 1 << 1,
    SYS_SAFE_MODE_ERROR_CAMERA = 1 << 2,
    SYS_SAFE_MODE_ERROR_BOOM = 1 << 3,
    SYS_SAFE_MODE_ERROR_GPS = 1 << 4,
    SYS_SAFE_MODE_ERROR_ADCS = 1 << 5,
    SYS_SAFE_MODE_ERROR_LFS = 1 << 6,
} SYS_safe_mode_error_enum_t;

uint8_t SYS_enter_safe_mode();
uint8_t SYS_eps_status_safe_mode_check();
uint8_t SYS_battery_safe_mode_check();
char* SYS_safe_mode_error_enum_to_string(SYS_safe_mode_error_enum_t error_mask);
uint8_t SYS_safe_mode_error_result_to_json(SYS_safe_mode_error_enum_t error, char *buffer, uint16_t buffer_size);

#endif // INCLUDE_GUARD__SYS_SAFE_MODE