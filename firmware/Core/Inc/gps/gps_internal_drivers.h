
#ifndef INCLUDE_GUARD__GPS_INTERNAL_DRIVERS_H__
#define INCLUDE_GUARD__GPS_INTERNAL_DRIVERS_H__

#include <stdint.h>

uint8_t GPS_send_cmd_get_response(const char *cmd_buf, uint8_t cmd_buf_len, uint8_t rx_buf[],
                                  uint16_t rx_buf_len, const uint16_t rx_buf_max_size);

uint8_t GPS_validate_log_response(const char *gps_response_buf);

#endif /* INCLUDE_GUARD__GPS_INTERNAL_DRIVERS_H__ */
