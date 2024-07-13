#ifndef __INCLUDE_GUARD_ANT_INTERNAL_DRIVERS_H__
#define __INCLUDE_GUARD_ANT_INTERNAL_DRIVERS_H__

#include <stdint.h>

// Pragma region constants

#define ANT_DEFAULT_RX_LEN_MIN 5 // for commands with no response params, 5 bytes are returned

uint8_t ANT_send_cmd_get_response(
    const uint8_t cmd_buf[], uint8_t cmd_len,
    uint8_t rx_buf[], uint16_t rx_len
    );

#endif /* __INCLUDE_GUARD_ANT_INTERNAL_DRIVERS_H__ */