#ifndef INCLUDE_GUARD__BULK_FILE_DOWNLINK_H__
#define INCLUDE_GUARD__BULK_FILE_DOWNLINK_H__

#include <stdint.h>

#include "littlefs/lfs.h" // For lfs_file_t type.
#include "littlefs/littlefs_helper.h" // For LFS_MAX_PATH_LENGTH

typedef enum {
    COMMS_BULK_FILE_DOWNLINK_STATE_IDLE,
    COMMS_BULK_FILE_DOWNLINK_STATE_DOWNLINKING,
    COMMS_BULK_FILE_DOWNLINK_STATE_PAUSED,
} COMMS_bulk_file_downlink_state_enum_t;


extern char COMMS_bulk_file_downlink_file_path[LFS_MAX_PATH_LENGTH];
extern uint32_t COMMS_bulk_file_downlink_absolute_start_offset;
extern uint32_t COMMS_bulk_file_downlink_next_start_offset;
extern uint32_t COMMS_bulk_file_downlink_total_bytes;
extern uint32_t COMMS_bulk_file_downlink_bytes_downlinked;
extern uint8_t COMMS_bulk_file_downlink_next_seq_num;
extern uint8_t COMMS_bulk_file_downlink_total_seq_num;
extern COMMS_bulk_file_downlink_state_enum_t COMMS_bulk_file_downlink_state;
extern lfs_file_t COMMS_bulk_file_downlink_file;


int32_t COMMS_bulk_file_downlink_start(char *file_path, uint32_t start_offset, uint32_t max_bytes);

uint8_t COMMS_bulk_file_downlink_pause(void);

uint8_t COMMS_bulk_file_downlink_resume(void);

#endif // INCLUDE_GUARD__BULK_FILE_DOWNLINK_H__
