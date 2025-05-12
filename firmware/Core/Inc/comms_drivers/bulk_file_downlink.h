#ifndef INCLUDE_GUARD__BULK_FILE_DOWNLINK_H__
#define INCLUDE_GUARD__BULK_FILE_DOWNLINK_H__

#include <stdint.h>

typedef enum {
    COMMS_BULK_FILE_DOWNLINK_STATE_IDLE,
    COMMS_BULK_FILE_DOWNLINK_STATE_DOWNLINKING,
    COMMS_BULK_FILE_DOWNLINK_STATE_PAUSED,
} COMMS_bulk_file_downlink_state_enum_t;


int32_t COMMS_bulk_file_downlink_start(char *file_path, uint32_t start_offset, uint32_t max_bytes);

uint8_t COMMS_bulk_file_downlink_pause(void);

uint8_t COMMS_bulk_file_downlink_resume(void);

#endif // INCLUDE_GUARD__BULK_FILE_DOWNLINK_H__
