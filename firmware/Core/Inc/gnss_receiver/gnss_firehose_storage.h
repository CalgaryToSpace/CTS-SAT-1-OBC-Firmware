#ifndef INCLUDE_GUARD__GNSS_RECEIVER_GNSS_FIREHOSE_STORAGE_H
#define INCLUDE_GUARD__GNSS_RECEIVER_GNSS_FIREHOSE_STORAGE_H

#include <stdint.h>
#include "littlefs/lfs.h"

extern uint8_t GNSS_firehose_file_is_open;
extern volatile uint32_t GNSS_firehose_bytes_lost;
extern lfs_file_t GNSS_firehose_file_pointer;
extern uint32_t GNSS_recording_start_uptime_ms;

uint8_t GNSS_enable_firehose_storage_mode(const char output_file_path[]);

uint8_t GNSS_disable_firehose_storage_mode(const char reason_for_stopping[]);


#endif // INCLUDE_GUARD__GNSS_RECEIVER_GNSS_FIREHOSE_STORAGE_H
