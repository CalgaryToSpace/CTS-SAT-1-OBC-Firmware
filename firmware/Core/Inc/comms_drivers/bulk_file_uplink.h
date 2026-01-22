#ifndef INCLUDE_GUARD_BULK_FILE_UPLINK_H
#define INCLUDE_GUARD_BULK_FILE_UPLINK_H

#include "littlefs/lfs.h"
#include <stdint.h>

typedef enum {
    COMMS_BULK_FILE_UPLINK_STATE_IDLE = 0,
    COMMS_BULK_FILE_UPLINK_STATE_OPEN
} COMMS_bulk_file_uplink_state_enum_t;

typedef enum {
    COMMS_BULK_FILE_UPLINK_MODE_TRUNCATE = 0,
    COMMS_BULK_FILE_UPLINK_MODE_APPEND
} COMMS_bulk_file_uplink_mode_enum_t;

int32_t COMMS_bulk_file_uplink_open_file(
    const char *file_path,
    COMMS_bulk_file_uplink_mode_enum_t mode
);

int32_t COMMS_bulk_file_uplink_write_bytes(const uint8_t bytes_to_write[], uint32_t bytes_to_write_length);

int32_t COMMS_bulk_file_uplink_seek(uint32_t new_position);

int32_t COMMS_bulk_file_uplink_close_file(void);

#endif // INCLUDE_GUARD_BULK_FILE_UPLINK_H
