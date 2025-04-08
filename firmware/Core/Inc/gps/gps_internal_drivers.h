
#ifndef INCLUDE_GUARD__GPS_INTERNAL_DRIVERS_H__
#define INCLUDE_GUARD__GPS_INTERNAL_DRIVERS_H__

#include <stdint.h>

uint8_t GPS_send_cmd_get_response(
    const char *cmd_buf, uint8_t cmd_buf_len,
    uint8_t rx_buf[],
    const uint16_t rx_buf_max_size,
    uint16_t* rx_buf_len_dest
);

uint8_t GPS_set_power_enabled(const char *cmd_arg, uint16_t enable_disable_flag, uint8_t rx_buf[],
                           uint16_t rx_buf_len, const uint16_t rx_buf_max_size);

#endif /* INCLUDE_GUARD__GPS_INTERNAL_DRIVERS_H__ */