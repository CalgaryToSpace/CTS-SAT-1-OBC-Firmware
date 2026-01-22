#include "comms_drivers/bulk_file_uplink.h"
#include "littlefs/littlefs_helper.h"
#include "log/log.h"

#include <string.h>
#include <stdio.h>
#include <ctype.h>

lfs_file_t COMMS_bulk_file_uplink_file;
char COMMS_bulk_file_uplink_file_path[LFS_MAX_PATH_LENGTH];
COMMS_bulk_file_uplink_state_enum_t COMMS_bulk_file_uplink_state =
    COMMS_BULK_FILE_UPLINK_STATE_IDLE;

// Conservative packet-size bound
#define COMMS_BULK_FILE_UPLINK_MAX_WRITE_BYTES 256


int32_t COMMS_bulk_file_uplink_open_file(
    const char *file_path,
    COMMS_bulk_file_uplink_mode_enum_t mode
) {
    // Check state.
    if (COMMS_bulk_file_uplink_state == COMMS_BULK_FILE_UPLINK_STATE_OPEN) {
        LOG_message(
            LOG_SYSTEM_LFS, LOG_SEVERITY_ERROR, LOG_all_sinks_except(LOG_SINK_FILE),
            "bulk_uplink_open_file: a file is already open!"
        );
        return 1;
    }

    int32_t flags = LFS_O_WRONLY | LFS_O_CREAT;
    if (mode == COMMS_BULK_FILE_UPLINK_MODE_TRUNCATE) {
        flags |= LFS_O_TRUNC;
    }
    else {
        flags |= LFS_O_APPEND;
    }

    const int32_t open_result = lfs_file_opencfg(
        &LFS_filesystem,
        &COMMS_bulk_file_uplink_file,
        file_path,
        flags,
        &LFS_file_cfg
    );
    if (open_result < 0) {
        LOG_message(
            LOG_SYSTEM_LFS, LOG_SEVERITY_ERROR, LOG_all_sinks_except(LOG_SINK_FILE),
            "bulk_uplink_open_file: lfs_file_opencfg() -> %ld",
            open_result
        );
        return open_result;
    }

    snprintf(
        COMMS_bulk_file_uplink_file_path,
        LFS_MAX_PATH_LENGTH,
        "%s",
        file_path
    );

    COMMS_bulk_file_uplink_state = COMMS_BULK_FILE_UPLINK_STATE_OPEN;

    LOG_message(
        LOG_SYSTEM_LFS, LOG_SEVERITY_NORMAL, LOG_all_sinks_except(LOG_SINK_FILE),
        "Bulk uplink opened file '%s'",
        file_path
    );

    return 0;
}

int32_t COMMS_bulk_file_uplink_write_bytes(const uint8_t bytes_to_write[], uint32_t bytes_to_write_length) {
    if (COMMS_bulk_file_uplink_state != COMMS_BULK_FILE_UPLINK_STATE_OPEN) {
        return 1; // File not open
    }
    
    const lfs_ssize_t write_result = lfs_file_write(
        &LFS_filesystem,
        &COMMS_bulk_file_uplink_file,
        bytes_to_write,
        bytes_to_write_length
    );

    if (write_result < 0 || (uint32_t)write_result != bytes_to_write_length) {
        LOG_message(
            LOG_SYSTEM_LFS, LOG_SEVERITY_ERROR, LOG_all_sinks_except(LOG_SINK_FILE),
            "bulk_uplink_write_bytes: lfs_file_write() -> %ld",
            write_result
        );
        return write_result;
    }

    return 0;
}

int32_t COMMS_bulk_file_uplink_seek(uint32_t new_position) {
    if (COMMS_bulk_file_uplink_state != COMMS_BULK_FILE_UPLINK_STATE_OPEN) {
        return 1;
    }

    const lfs_soff_t seek_result = lfs_file_seek(
        &LFS_filesystem,
        &COMMS_bulk_file_uplink_file,
        new_position,
        LFS_SEEK_SET
    );

    if (seek_result < 0) {
        LOG_message(
            LOG_SYSTEM_LFS, LOG_SEVERITY_ERROR, LOG_all_sinks_except(LOG_SINK_FILE),
            "bulk_uplink_seek: lfs_file_seek() -> %ld",
            seek_result
        );
        return seek_result;
    }

    return 0;
}

int32_t COMMS_bulk_file_uplink_close_file(void) {
    if (COMMS_bulk_file_uplink_state != COMMS_BULK_FILE_UPLINK_STATE_OPEN) {
        LOG_message(
            LOG_SYSTEM_LFS, LOG_SEVERITY_WARNING, LOG_all_sinks_except(LOG_SINK_FILE),
            "bulk_uplink_close_file: no file is open"
        );
        return 1; // If no file is open, indicate as such.
    }

    const int32_t close_result =
        lfs_file_close(&LFS_filesystem, &COMMS_bulk_file_uplink_file);

    if (close_result != 0) {
        LOG_message(
            LOG_SYSTEM_LFS, LOG_SEVERITY_WARNING, LOG_all_sinks_except(LOG_SINK_FILE),
            "bulk_uplink_close_file: lfs_file_close() -> %ld",
            close_result
        );
        // Note: Steamroll here.
    }

    COMMS_bulk_file_uplink_state = COMMS_BULK_FILE_UPLINK_STATE_IDLE;
    return 0;
}
