
#ifndef INCLUDE_GUARD__GNSS_INTERNAL_DRIVERS_H__
#define INCLUDE_GUARD__GNSS_INTERNAL_DRIVERS_H__

#include <stdint.h>

typedef enum {
    GNSS_RX_MODE_COMMAND_MODE,   // GNSS is in command mode.
    GNSS_RX_MODE_FIREHOSE_MODE,  // GNSS is in firehose data storage mode.
    GNSS_RX_MODE_DISABLED        // GNSS data is not being collected/stored.
} GNSS_rx_mode_enum_t;

extern GNSS_rx_mode_enum_t GNSS_current_rx_mode;

uint8_t GNSS_send_cmd_get_response(
    const char *cmd_buf, uint8_t cmd_buf_len,
    uint8_t rx_buf[],
    const uint16_t rx_buf_max_size,
    uint16_t* rx_buf_len_dest,
    uint8_t remove_null_bytes_in_middle
);

const char* GNSS_rx_mode_enum_to_str(GNSS_rx_mode_enum_t rx_mode);

#endif /* INCLUDE_GUARD__GNSS_INTERNAL_DRIVERS_H__ */