
#ifndef INCLUDE_GUARD__GPS_INTERNAL_DRIVERS_H__
#define INCLUDE_GUARD__GPS_INTERNAL_DRIVERS_H__

#include <stdint.h>

#define GPS_DEFAULT_RX_LEN_MIN 9 // every GPS log command response has a structure of "<{STATUS}\n[COM{#}]" eg "<OK\n[COM1]", 9 bytes minimum
#define EPS_ENABLE_DEBUG_PRINT 1 // bool; 0 to disable

uint8_t GPS_send_cmd_get_response(const char cmd_buf[], uint8_t cmd_buf_len,
        uint8_t rx_buf[], uint16_t rx_buf_len);


#endif /* INCLUDE_GUARD__GPS_INTERNAL_DRIVERS_H__ */
