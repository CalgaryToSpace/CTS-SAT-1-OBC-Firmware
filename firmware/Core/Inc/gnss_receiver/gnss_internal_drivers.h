#ifndef INCLUDE_GUARD__GNSS_INTERNAL_DRIVERS_H__
#define INCLUDE_GUARD__GNSS_INTERNAL_DRIVERS_H__

#include <stdint.h>

uint8_t GNSS_send_cmd_get_response(
    const char *cmd_buf, uint8_t cmd_buf_len,
    uint8_t rx_buf[],
    const uint16_t rx_buf_max_size,
    uint16_t* rx_buf_len_dest
);

#endif /* INCLUDE_GUARD__GNSS_INTERNAL_DRIVERS_H__ */