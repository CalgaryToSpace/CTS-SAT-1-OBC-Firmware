// GNSS Firehose Storage
// This module is responsible for storing GNSS data received at any point to a file.
// It is in contrast with the nominal "command mode" operation, where the GNSS receiver only
// replies to commands sent by the OBC (e.g., `log bestxyza once`). In "firehose mode", the
// OBC stores all data received from the GNSS receiver to a file; it expects that the GNSS
// receiver is configured in a `log bestxyza ontime 100` mode or similar.

#include "gnss_receiver/gnss_firehose_storage.h"

#include "gnss_receiver/gnss_internal_drivers.h"
#include "uart_handler/uart_handler.h"
#include "log/log.h"
#include "littlefs/littlefs_helper.h"
#include "timekeeping/timekeeping.h"
#include "transforms/arrays.h"

#include <stdint.h>
#include <stdio.h>

uint8_t GNSS_firehose_file_is_open = 0;
volatile uint32_t GNSS_firehose_bytes_lost = 0;
lfs_file_t GNSS_firehose_file_pointer;
uint32_t GNSS_recording_start_uptime_ms = 0;


static int8_t GNSS_write_firehose_file_header() {
    // Write timestamp data (the time the buffer finished filling) to file.
    const uint32_t uptime_ms = TIME_uptime_ms();

    const uint64_t timestamp_ms = TIME_convert_uptime_to_unix_epoch_time_ms(uptime_ms);
    char timestamp_ms_str[32];
    GEN_uint64_to_str(timestamp_ms, timestamp_ms_str);

    char datetime_fmt_str[40];
    TIME_format_utc_datetime_str(
        datetime_fmt_str, sizeof(datetime_fmt_str),
        timestamp_ms, TIME_last_synchronization_source
    );

    char timestamp_fmt_str[40];
    TIME_format_timestamp_str(
        timestamp_fmt_str, sizeof(timestamp_fmt_str),
        timestamp_ms, TIME_last_synchronization_source
    );

    char header_str[200];
    snprintf(
        header_str, sizeof(header_str),
        "{\"gnss_start\":1,\"uptime_ms\":%ld,\"timestamp\":\"%s\",\"datetime\":\"%s\",\"timestamp_ms\":%s}\n",
        uptime_ms,
        timestamp_fmt_str,
        datetime_fmt_str,
        timestamp_ms_str
    );

    const lfs_ssize_t write_timestamp_result = lfs_file_write(
        &LFS_filesystem, &GNSS_firehose_file_pointer,
        header_str, strlen(header_str)
    );
    if (write_timestamp_result < 0) {
        LOG_message(
            LOG_SYSTEM_GNSS, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
            "GNSS Header: Error writing timestamp to file: %ld", write_timestamp_result
        );
        return write_timestamp_result;
    }

    LOG_message(
        LOG_SYSTEM_GNSS, LOG_SEVERITY_NORMAL, LOG_all_sinks_except(LOG_SINK_FILE),
        "GNSS Header: File header written successfully"
    );
    return 0; // Success.
}


/// @brief Write the end-of-recording JSON data to the file.
/// @return 0 on success.
/// @note The normal timestamp function is called right before this function, so no need to include timestamps in here.
static int8_t GNSS_write_firehose_file_footer(const char reason_for_stopping[]) {
    char buffer_footer_str[200];
    snprintf(
        buffer_footer_str, sizeof(buffer_footer_str),
        "\n{\"data_lost_bytes\": %lu, \"time_taken_ms\": %lu, \"reason_for_stopping\": \"%s\" }",
        GNSS_firehose_bytes_lost,
        (TIME_uptime_ms() - GNSS_recording_start_uptime_ms),
        reason_for_stopping
    );

    const lfs_ssize_t write_timestamp_result = lfs_file_write(
        &LFS_filesystem, &GNSS_firehose_file_pointer,
        buffer_footer_str, strlen(buffer_footer_str)
    );
    if (write_timestamp_result < 0) {
        LOG_message(
            LOG_SYSTEM_MPI, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
            "MPI Header: Error writing footer to file: %ld", write_timestamp_result
        );
        return write_timestamp_result;
    }
    return 0; // Success
}


/// @brief Opens a file to store incoming GNSS data.
/// @return 0: System successfully prepared for GNSS data, < 0: Error
static int8_t GNSS_prepare_to_receive_data(const char output_file_path[]) {
    // Mount LFS if not already mounted
    if (!LFS_is_lfs_mounted) {
        LFS_mount();
    }

    // If the old file is open, close the file
    if (GNSS_firehose_file_is_open == 1) {
        const int8_t close_result = lfs_file_close(&LFS_filesystem, &GNSS_firehose_file_pointer);
        if (close_result < 0) {
            LOG_message(
                LOG_SYSTEM_GNSS, LOG_SEVERITY_WARNING, LOG_all_sinks_except(LOG_SINK_FILE),
                "Error closing old file: %d", close_result
            );
            return close_result;
        }
        LOG_message(
            LOG_SYSTEM_GNSS, LOG_SEVERITY_WARNING, LOG_all_sinks_except(LOG_SINK_FILE),
            "Old File successfully closed"
        );
        GNSS_firehose_file_is_open = 0;
    }

    // Open / Create the file
    const int8_t open_result = lfs_file_open(
        &LFS_filesystem, &GNSS_firehose_file_pointer,
        output_file_path,
        LFS_O_WRONLY | LFS_O_CREAT | LFS_O_APPEND
    );
    
    // Check if open successful
    if (open_result < 0) {
        LOG_message(
            LOG_SYSTEM_GNSS, LOG_SEVERITY_WARNING, LOG_all_sinks_except(LOG_SINK_FILE),
            "Error opening / creating file: %s", output_file_path
        );
        return open_result;
    }

    // Change the state to state file is open
    GNSS_firehose_file_is_open = 1;
    LOG_message(
        LOG_SYSTEM_GNSS, LOG_SEVERITY_NORMAL, LOG_all_sinks_except(LOG_SINK_FILE),
        "Opened/created file: %s", output_file_path
    );

    const int8_t header_status = GNSS_write_firehose_file_header();
    if (header_status < 0) {
        LOG_message(
            LOG_SYSTEM_GNSS, LOG_SEVERITY_WARNING, LOG_all_sinks_except(LOG_SINK_FILE),
            "Error writing file header: %d", header_status
        );
        return header_status; // Error writing file header
    }
    
    return 0;
}


/// @brief Configures the OBC to receive and store all data from the GNSS receiver to a file.
/// @return 0: Success. Non-zero: Error.
uint8_t GNSS_enable_firehose_storage_mode(const char output_file_path[]) {
    // Turn on the MPI and setup LFS
    const uint8_t prepare_result = GNSS_prepare_to_receive_data(output_file_path);
    if (prepare_result != 0) {
        LOG_message(LOG_SYSTEM_GNSS, LOG_SEVERITY_ERROR, LOG_SINK_ALL, 
            "MPI could not be powered on (MPI_prepare_receive_data err: %d)", prepare_result);
        return 5;
    }
    
    // Make it start writing to the start of the buffer.
    UART_gnss_buffer_write_idx = 0;
    GNSS_current_rx_mode = GNSS_RX_MODE_FIREHOSE_MODE;

    // Enable GNSS receiving.
    GNSS_set_uart_interrupt_state(1);
    
    // Init counters, etc.
    GNSS_firehose_bytes_lost = 0;
    GNSS_recording_start_uptime_ms = TIME_uptime_ms();

    return 0;
}

uint8_t GNSS_disable_firehose_storage_mode(const char reason_for_stopping[]) {
    // Disable GNSS receiving.
    // Safety: Very important to do this reliably and immediately to avoid GNSS sending
    // eradic/noisy interrupts when unpowered.
    GNSS_set_uart_interrupt_state(0);

    if (GNSS_current_rx_mode != GNSS_RX_MODE_FIREHOSE_MODE) {
        LOG_message(
            LOG_SYSTEM_GNSS, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
            "MPI stop command called when not currently in sensing mode. Can't close file."
        );
        GNSS_current_rx_mode = GNSS_RX_MODE_DISABLED; // Reset the state.
        return 1;
    }

    GNSS_current_rx_mode = GNSS_RX_MODE_DISABLED; // Set UART mode to not listening.

    GNSS_write_firehose_file_footer(reason_for_stopping);

    // Close the file. The file in storage is not updated until the file is closed successfully.
    const int8_t close_result = lfs_file_close(&LFS_filesystem, &GNSS_firehose_file_pointer);
    GNSS_firehose_file_is_open = 0;
    if (close_result < 0) {
        LOG_message(
            LOG_SYSTEM_GNSS, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "MPI stop: Error closing file: %d", close_result
        );
        return 2;
    }

    // Log that file has been successfully closed 
    LOG_message(
        LOG_SYSTEM_GNSS, LOG_SEVERITY_DEBUG, LOG_SINK_ALL,
        "GNSS firehose stop: File closed successfully"
    );

    // Get the file size before closing the file
    const lfs_ssize_t file_size = lfs_file_size(&LFS_filesystem, &GNSS_firehose_file_pointer);
    if (file_size < 0) {
        LOG_message(
            LOG_SYSTEM_GNSS, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "GNSS firehose stop: Error getting file size: %ld", file_size
        );
    }
    
    // Log GNSS data stats (very similar to ones stored in the footer).
    LOG_message(
        LOG_SYSTEM_GNSS, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
        "{\"data_stored_bytes\": %ld, \"data_lost_bytes\": %lu, \"time_taken_ms\": %lu, \"reason_for_stopping\": \"%s\" }",
        file_size,
        GNSS_firehose_bytes_lost,
        TIME_uptime_ms() - GNSS_recording_start_uptime_ms,
        reason_for_stopping
    );

    return 0;
}

static uint32_t last_gnss_flush_uptime_ms = 0;

// We don't have an auto-stop on the GNSS storage, so we'll full it periodically in case of crash.
uint32_t GNSS_firehose_flush_interval_ms = 30000;


/// @brief Store pending data in the UART_gnss_buffer to the GNSS firehose file.
/// @return 0 on success or no-op. Non-zero: Error.
/// @note Intended to be called in a periodic background loop, but also can be called from telecommand.
uint8_t GNSS_subtask_store_firehose_data_to_file() {
    // This function is intended to only run when in firehose mode.
    if (GNSS_current_rx_mode != GNSS_RX_MODE_FIREHOSE_MODE) {
        return 0;
    }

    // If there's no data to write, early exit.
    if (UART_gnss_buffer_write_idx == 0) {
        return 0;
    }

    // If there's no file to write to, early exit.
    if (!GNSS_firehose_file_is_open) {
        return 0;
    }


    // If the response is short (bestxyza, versiona, etc.), copy to a buffer. If it's too large,
    // (e.g., ephema), then we'll just read it right out of the UART buffer, and make the assumption
    // that there's a low chance the buffer will change while we're reading out of it.
    const uint16_t gnss_rx_data_len_max_for_copy = 512; // Max stack-allocated buffer.
    const uint16_t gnss_rx_data_len = UART_gnss_buffer_write_idx;
    uint8_t gnss_rx_data_if_short[gnss_rx_data_len_max_for_copy]; // Unused if data is long.
    uint8_t* gnss_rx_data_ptr;
    if (gnss_rx_data_len <= gnss_rx_data_len_max_for_copy) {
        // memcpy from the volatile:
        for (uint16_t i = 0; i < gnss_rx_data_len; i++) { // Memcpy out of volatile.
            gnss_rx_data_if_short[i] = UART_gnss_buffer[i];
        }
        gnss_rx_data_ptr = gnss_rx_data_if_short;
    }
    else {
        // Too long to copy to the stack.
        gnss_rx_data_ptr = (uint8_t*) UART_gnss_buffer; // Discard volatile.
    }

    // Reset the write index so subsequent writes go back to the start.
    UART_gnss_buffer_write_idx = 0;

    // Write to the file.
    const lfs_ssize_t write_result = lfs_file_write(
        &LFS_filesystem, &GNSS_firehose_file_pointer,
        gnss_rx_data_ptr, // Either `UART_gnss_buffer` directly, or a stack-allocated buffer.
        gnss_rx_data_len // Copy of UART_gnss_buffer_write_idx
    );
    if (write_result < 0) {
        LOG_message(
            LOG_SYSTEM_GNSS, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "GNSS firehose: Error writing to file: %ld", write_result
        );
        return 10;
    }

    LOG_message(
        LOG_SYSTEM_GNSS, LOG_SEVERITY_DEBUG, LOG_SINK_ALL,
        "GNSS firehose: Successfully wrote %ld bytes.",
        write_result
    );

    // Conditionally, flush the file to storage.
    if (TIME_uptime_ms() - last_gnss_flush_uptime_ms > GNSS_firehose_flush_interval_ms) {
        last_gnss_flush_uptime_ms = TIME_uptime_ms();
        const int8_t flush_result = lfs_file_sync(&LFS_filesystem, &GNSS_firehose_file_pointer);
        if (flush_result < 0) {
            LOG_message(
                LOG_SYSTEM_GNSS, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
                "GNSS firehose: Error flushing file: %d", flush_result
            );
            return 11;
        }

        LOG_message(
            LOG_SYSTEM_GNSS, LOG_SEVERITY_DEBUG, LOG_SINK_ALL,
            "GNSS firehose: Flushed file."
        );
    }

    // Success.
    return 0;
}

