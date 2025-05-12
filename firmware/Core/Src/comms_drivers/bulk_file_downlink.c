#include "comms_drivers/bulk_file_downlink.h"
#include "littlefs/lfs.h"
#include "littlefs/littlefs_helper.h"
#include "log/log.h"
#include "comms_drivers/comms_tx.h" // For `COMMS_bulk_file_downlink_next_seq_num`

#include <string.h>

// Note: These are all externs:
char COMMS_bulk_file_downlink_file_path[LFS_MAX_PATH_LENGTH];
uint32_t COMMS_bulk_file_downlink_absolute_start_offset;
uint32_t COMMS_bulk_file_downlink_next_start_offset;
uint32_t COMMS_bulk_file_downlink_total_bytes;
uint32_t COMMS_bulk_file_downlink_bytes_downlinked;
uint8_t COMMS_bulk_file_downlink_next_seq_num;
uint8_t COMMS_bulk_file_downlink_total_seq_num;
COMMS_bulk_file_downlink_state_enum_t COMMS_bulk_file_downlink_state;
lfs_file_t COMMS_bulk_file_downlink_file;

static const uint32_t COMMS_bulk_file_downlink_max_allowable_total_bytes = 1000000;

/// @brief Initiate a bulk file downlink over the UHF radio.
/// @param file_path File name/path to downlink.
/// @param start_offset The byte offset in the file to start downlinking from (0 for start).
/// @param max_bytes The maximum number of bytes to downlink. Maximum value is 1000000 (1 MB, COMMS_bulk_file_downlink_total_bytes), for safety (to avoid a very very long-running downlink chain). Values >1 MB will be limited to 1 MB, and value 0 will be set to 1 MB.
/// @return 0 on success. Negative LFS failure code on LFS failure. Positive error code on other (logical/request) errors.
/// @note 1 MB takes about 15 minutes to downlink at 9600 baud.
/// @note This function is safe to call at any point (including mid-downlink, or mid-pause).
///       It will close the previous file and start a new downlink.
int32_t COMMS_bulk_file_downlink_start(char *file_path, uint32_t start_offset, uint32_t max_bytes) {
    // Check clean-up actions.
    if (
        (COMMS_bulk_file_downlink_state == COMMS_BULK_FILE_DOWNLINK_STATE_DOWNLINKING)
        || (COMMS_bulk_file_downlink_state == COMMS_BULK_FILE_DOWNLINK_STATE_PAUSED)
    ) {
        // Must return to idle.
        // Close the file.
        const int8_t close_status = lfs_file_close(&LFS_filesystem, &COMMS_bulk_file_downlink_file);
        if (close_status != 0) {
            LOG_message(
                LOG_SYSTEM_LFS, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
                "During COMMS_bulk_file_downlink_start idle return, lfs_file_close()->%d",
                close_status
            );
            // Steamroll (contain on anyway).
        }

        COMMS_bulk_file_downlink_state = COMMS_BULK_FILE_DOWNLINK_STATE_IDLE;
    }

    // Open the new file. Implicitly checks if it exists.
    const int32_t open_result = lfs_file_opencfg(
        &LFS_filesystem, &COMMS_bulk_file_downlink_file, file_path, LFS_O_RDONLY, &LFS_file_cfg
    );
    if (open_result < 0) {
        LOG_message(
            LOG_SYSTEM_LFS, LOG_SEVERITY_ERROR, LOG_all_sinks_except(LOG_SINK_FILE),
            "COMMS_bulk_file_downlink_start lfs_file_opencfg()->%ld",
            open_result
        );
        return open_result;
    }
    LOG_message(
        LOG_SYSTEM_LFS, LOG_SEVERITY_DEBUG,
        LOG_all_sinks_except(LOG_SINK_FILE),
        "Successfully opened file for downlink."
    );

    // Fetch filesize.
    const lfs_ssize_t size_bytes = lfs_file_size(&LFS_filesystem, &COMMS_bulk_file_downlink_file);
    if (size_bytes < 0) {
        LOG_message(
            LOG_SYSTEM_LFS, LOG_SEVERITY_ERROR, LOG_all_sinks_except(LOG_SINK_FILE),
            "COMMS_bulk_file_downlink_start lfs_file_size()->%ld",
            size_bytes
        );
        return size_bytes;
    }

    // Validate parameter: start_offset.
    if (((int32_t)start_offset) >= size_bytes) {
        LOG_message(
            LOG_SYSTEM_LFS, LOG_SEVERITY_ERROR, LOG_all_sinks_except(LOG_SINK_FILE),
            "COMMS_bulk_file_downlink_start: start_offset %ld bytes >= file size %ld bytes",
            start_offset, size_bytes
        );
        return 3; // Invalid offset
    }

    // Validate/restrict parameter: max_bytes.
    if (max_bytes > COMMS_bulk_file_downlink_max_allowable_total_bytes) {
        LOG_message(
            LOG_SYSTEM_LFS, LOG_SEVERITY_WARNING, LOG_all_sinks_except(LOG_SINK_FILE),
            "COMMS_bulk_file_downlink_start: max_bytes %ld being restricted to 1 MB (1,000,000 bytes)",
            max_bytes
        );
        max_bytes = COMMS_bulk_file_downlink_max_allowable_total_bytes;
    }
    else if (max_bytes == 0) {
        LOG_message(
            LOG_SYSTEM_LFS, LOG_SEVERITY_WARNING, LOG_all_sinks_except(LOG_SINK_FILE),
            "COMMS_bulk_file_downlink_start: max_bytes %ld being set to 1 MB (1,000,000 bytes)",
            max_bytes
        );
        max_bytes = COMMS_bulk_file_downlink_max_allowable_total_bytes;
    }

    // Validate/restrict parameter: max_bytes (Round 2: based on total file size).
    if (max_bytes > (size_bytes - start_offset)) {
        LOG_message(
            LOG_SYSTEM_LFS, LOG_SEVERITY_NORMAL, LOG_all_sinks_except(LOG_SINK_FILE),
            "COMMS_bulk_file_downlink_start: Downlinking the result of the file by setting max_bytes to %ld bytes",
            size_bytes - start_offset
        );
        max_bytes = size_bytes - start_offset;
    }

    // Seek the file to the start offset.
    const int32_t seek_result = lfs_file_seek(
        &LFS_filesystem, &COMMS_bulk_file_downlink_file, start_offset, LFS_SEEK_SET
    );
    if (seek_result < 0) {
        LOG_message(
            LOG_SYSTEM_LFS, LOG_SEVERITY_ERROR, LOG_all_sinks_except(LOG_SINK_FILE),
            "COMMS_bulk_file_downlink_start lfs_seek()->%ld",
            seek_result
        );
        return seek_result;
    }

    // Set the parameters globally.
    COMMS_bulk_file_downlink_absolute_start_offset = start_offset;
    COMMS_bulk_file_downlink_next_start_offset = start_offset;
    COMMS_bulk_file_downlink_total_bytes = max_bytes;
    COMMS_bulk_file_downlink_bytes_downlinked = 0;
    COMMS_bulk_file_downlink_next_seq_num = 1; // First packet is 1.
    COMMS_bulk_file_downlink_total_seq_num = (
        (max_bytes + COMMS_BULK_FILE_DOWNLINK_PACKET_MAX_DATA_BYTES_PER_PACKET - 1)
        / COMMS_BULK_FILE_DOWNLINK_PACKET_MAX_DATA_BYTES_PER_PACKET
    );
    strncpy(COMMS_bulk_file_downlink_file_path, file_path, LFS_MAX_PATH_LENGTH);

    // Finally, set the new state to activate it.
    COMMS_bulk_file_downlink_state = COMMS_BULK_FILE_DOWNLINK_STATE_DOWNLINKING;
    
    return 0; // Return 0 for success
}

uint8_t COMMS_bulk_file_downlink_pause(void) {
    if (COMMS_bulk_file_downlink_state == COMMS_BULK_FILE_DOWNLINK_STATE_IDLE) {
        // Can't pause if it's not running. A resume would cause it to start downlinking invalid data.
        return 1; // Return 1 for failure
    }

    COMMS_bulk_file_downlink_state = COMMS_BULK_FILE_DOWNLINK_STATE_PAUSED;
    return 0; // Return 0 for success    
}


uint8_t COMMS_bulk_file_downlink_resume(void) {
    if (COMMS_bulk_file_downlink_state != COMMS_BULK_FILE_DOWNLINK_STATE_PAUSED) {
        // Can't resume if it's not paused.
        return 1;
    }

    COMMS_bulk_file_downlink_state = COMMS_BULK_FILE_DOWNLINK_STATE_DOWNLINKING;
    return 0; // Return 0 for success    
}

