#include "rtos_tasks/rtos_bulk_downlink_task.h"
#include "rtos_tasks/rtos_task_helpers.h"

#include "comms_drivers/bulk_file_downlink.h"
#include "comms_drivers/comms_tx.h"
#include "littlefs/lfs.h"
#include "littlefs/littlefs_helper.h"
#include "log/log.h"

#include <string.h>

/// @brief The period to wait between downlink packets.
/// @note A 250-byte packet at 9600 baud takes about 208 ms to transmit.
/// @example If you reconfigure the AX100 and increase the baudrate of the radio, decrease this value.
uint32_t COMMS_bulk_downlink_delay_per_packet_ms = 208;

static uint8_t bulk_file_data[COMMS_BULK_FILE_DOWNLINK_PACKET_MAX_DATA_BYTES_PER_PACKET];

/// @brief Downlink task action.
/// @param  
/// @note Expects that the state has already been checked to be COMMS_BULK_FILE_DOWNLINK_STATE_DOWNLINKING.
static void do_bulk_downlink_task_action(void) {
    if (COMMS_bulk_file_downlink_state != COMMS_BULK_FILE_DOWNLINK_STATE_DOWNLINKING) {
        return; // Safety check
    }
    
    // Read the file data.
    const uint32_t bytes_remaining = COMMS_bulk_file_downlink_total_bytes - COMMS_bulk_file_downlink_bytes_downlinked;
    const uint16_t byte_count = 
        bytes_remaining > COMMS_BULK_FILE_DOWNLINK_PACKET_MAX_DATA_BYTES_PER_PACKET
        ? COMMS_BULK_FILE_DOWNLINK_PACKET_MAX_DATA_BYTES_PER_PACKET
        : bytes_remaining;

    const lfs_ssize_t read_result = lfs_file_read(
        &LFS_filesystem,
        &COMMS_bulk_file_downlink_file,
        bulk_file_data,
        byte_count
    );
    if (read_result < 0) {
        LOG_message(
            LOG_SYSTEM_LFS, LOG_SEVERITY_ERROR, LOG_all_sinks_except(LOG_SINK_FILE),
            "lfs_read(byte_count=%d) -> read_result=%ld",
            byte_count,
            read_result
        );
        return;
    }

    // Downlink the data.
    const uint8_t tx_result = COMMS_downlink_bulk_file_downlink(
        COMMS_bulk_file_downlink_next_seq_num,
        COMMS_bulk_file_downlink_total_seq_num,
        COMMS_bulk_file_downlink_next_start_offset,
        bulk_file_data,
        byte_count
    );
    if (tx_result != 0) {
        LOG_message(
            LOG_SYSTEM_LFS, LOG_SEVERITY_ERROR, LOG_all_sinks_except(LOG_SINK_FILE),
            "COMMS_TX_send_bytes(byte_count=%d) -> tx_result=%d",
            byte_count,
            tx_result
        );
        return;
    }

    // Update all the downlink stats.
    COMMS_bulk_file_downlink_bytes_downlinked += byte_count;
    COMMS_bulk_file_downlink_next_seq_num++;
    COMMS_bulk_file_downlink_next_start_offset += byte_count;

    // Check next-state logic if we're done.
    if (COMMS_bulk_file_downlink_bytes_downlinked == COMMS_bulk_file_downlink_total_bytes) {
        COMMS_bulk_file_downlink_state = COMMS_BULK_FILE_DOWNLINK_STATE_IDLE;

        LOG_message(
            LOG_SYSTEM_LFS, LOG_SEVERITY_NORMAL, LOG_all_sinks_except(LOG_SINK_FILE),
            "Bulk downlink complete. %ld bytes (%d packets) downlinked. File: %s",
            COMMS_bulk_file_downlink_bytes_downlinked,
            COMMS_bulk_file_downlink_total_seq_num,
            COMMS_bulk_file_downlink_file_path
        );
    }
}

void TASK_bulk_downlink(void *argument) {
    TASK_HELP_start_of_task();
    while(1) {
        if (
            (COMMS_bulk_file_downlink_state == COMMS_BULK_FILE_DOWNLINK_STATE_IDLE)
            || (COMMS_bulk_file_downlink_state == COMMS_BULK_FILE_DOWNLINK_STATE_PAUSED)
        ) {
            osDelay(500);
            continue;
        }
        else if (COMMS_bulk_file_downlink_state == COMMS_BULK_FILE_DOWNLINK_STATE_DOWNLINKING) {
            do_bulk_downlink_task_action();

            // Delay to avoid flooding the radio with packets.
            // The AX100 seems to have a small queue, but can be overwhelmed easily.
            osDelay(COMMS_bulk_downlink_delay_per_packet_ms);
        }
        else {
            LOG_message(
                LOG_SYSTEM_LFS, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
                "Unknown COMMS_bulk_file_downlink_state %d",
                COMMS_bulk_file_downlink_state
            );
            osDelay(500);
            continue;
        }
    }
}
