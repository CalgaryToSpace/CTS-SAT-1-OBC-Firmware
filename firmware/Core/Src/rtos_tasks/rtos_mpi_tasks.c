#include "rtos_tasks/rtos_task_helpers.h"
#include "mpi/mpi_command_handling.h"
#include "littlefs/littlefs_helper.h"
#include "debug_tools/debug_uart.h"
#include "cmsis_os.h"
#include "log/log.h"
#include "uart_handler/uart_handler.h"
#include "timekeeping/timekeeping.h"
#include "transforms/arrays.h"

#include <stdio.h>

static int8_t write_mpi_timestamp_to_file(uint32_t buffer_filled_uptime_ms) {
    // Write timestamp data (the time the buffer finished filling) to file.
    const uint32_t uptime_ms = TIME_get_current_system_uptime_ms();

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

    char buffer_footer_str[200];
    snprintf(
        buffer_footer_str, sizeof(buffer_footer_str),
        "{\"uptime_ms\":%ld,\"timestamp\":\"%s\",\"datetime\":\"%s\",\"timestamp_ms\":%s}",
        uptime_ms,
        timestamp_fmt_str,
        datetime_fmt_str,
        timestamp_ms_str
    );

    const lfs_ssize_t write_timestamp_result = lfs_file_write(
        &LFS_filesystem, &MPI_science_data_file_pointer,
        buffer_footer_str, strlen(buffer_footer_str)
    );
    if (write_timestamp_result < 0) {
        LOG_message(
            LOG_SYSTEM_MPI, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
            "MPI Header: Error writing timestamp to file: %ld", write_timestamp_result
        );
        return write_timestamp_result;
    }
    return 0; // Success
}

static void write_mpi_data_to_memory(volatile uint8_t* large_buffer, uint32_t buffer_filled_uptime_ms) {
    // Store the current time for this iteration
    const uint32_t start_time = HAL_GetTick();

    // Ensure LFS is mounted. Steamroll.
    LFS_ensure_mounted();

    // Write science data to file.
    const lfs_ssize_t write_data_result = lfs_file_write(
        &LFS_filesystem, &MPI_science_data_file_pointer,
        (uint8_t*)large_buffer, MPI_science_buffer_len
    );
    if (write_data_result < 0) {
        LOG_message(
            LOG_SYSTEM_MPI, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
            "MPI Task: Error writing to file: %ld", write_data_result
        );
        return; // Exit early if write failed
    }

    const int8_t write_timestamp_result = write_mpi_timestamp_to_file(buffer_filled_uptime_ms);
    if (write_timestamp_result < 0) {
        LOG_message(
            LOG_SYSTEM_MPI, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
            "MPI Task: Error writing timestamp to file: %d", write_timestamp_result
        );
        return; // Exit early if timestamp write failed
    }

    LOG_message(
        LOG_SYSTEM_MPI, LOG_SEVERITY_DEBUG, LOG_SINK_ALL,
        "MPI Task: Successfully wrote %ld bytes to file in %lums",
        write_data_result,
        HAL_GetTick() - start_time
    );
}


void TASK_service_write_mpi_data(void *argument) {
    TASK_HELP_start_of_task();
    osDelay(5000);

    while(1) {
        // If the first large buffer contains data to write
        if (MPI_buffer_one_state == MPI_MEMORY_WRITE_STATUS_AWAITING_WRITE) {
            write_mpi_data_to_memory(
                MPI_science_buffer_one,
                MPI_buffer_one_last_filled_uptime_ms
            );

            MPI_buffer_one_state = MPI_MEMORY_WRITE_STATUS_READY_TO_FILL;
        }
        else if (MPI_buffer_two_state == MPI_MEMORY_WRITE_STATUS_AWAITING_WRITE) {
            write_mpi_data_to_memory(
                MPI_science_buffer_two,
                MPI_buffer_one_last_filled_uptime_ms
            );

            MPI_buffer_two_state = MPI_MEMORY_WRITE_STATUS_READY_TO_FILL;
        }
        
        // Do a short delay to allow recording to start right away once enabled.
        osDelay(100);
    }
}