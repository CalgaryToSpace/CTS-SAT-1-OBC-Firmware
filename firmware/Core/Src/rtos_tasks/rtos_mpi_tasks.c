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

/// @brief If the MPI exceeds this value during data recording, recording will be disabled.
/// @note Set this value to 0 (via configuration) to disable the temperature shutoff feature.
uint32_t MPI_max_temperature_shutoff_celcius = 60;

/// @brief Maximum duration for MPI data recording in seconds.
/// @note Set this value to 0 (via configuration) to disable the duration limit.
/// @note This configuration variable is designed to prevent against an operator failing to
///        schedule a stop telecommand for MPI data recording.
uint32_t MPI_max_recording_duration_sec = 900;


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

/// @brief Scan an MPI data buffer, averaging all temperature reports in it.
/// @param large_buffer MPI data buffer input.
/// @return Average temperature in 100ths of a degree Celsius (cC). Returns special value -9999 on error.
int32_t read_avg_temperature_cC_from_mpi_data_buffer(
    volatile uint8_t* large_buffer
) {
    const uint8_t sync_pattern[4] = {0x0c, 0xff, 0xff, 0x0c};

    int64_t temp_sum_centi = 0;  // Use int64 to prevent overflow.
    uint32_t temp_count = 0;

    for (uint32_t i = 0; i + 7 < MPI_science_buffer_len; i++) {
        // Check sync pattern.
        if (large_buffer[i + 0] == sync_pattern[0] &&
            large_buffer[i + 1] == sync_pattern[1] &&
            large_buffer[i + 2] == sync_pattern[2] &&
            large_buffer[i + 3] == sync_pattern[3]
        ) {
            // Ensure temperature bytes are inside buffer.
            if (i + 7 < MPI_science_buffer_len) {
                // Assumption: Negative values will be handled gracefully by C, just
                // by storing the value in a signed int.
                const int16_t raw_temp =
                    ((int16_t)large_buffer[i + 6] << 8) |
                    (int16_t)large_buffer[i + 7];

                // Convert to centi-Celsius (https://github.com/CalgaryToSpace/CTS-SAT-1-OBC-Firmware/issues/462):
                // Celsius = raw_temp / 128.0
                // centi-Celsius = (raw_temp * 100) / 128
                const int32_t temp_centi = (raw_temp * 100U) / 128U;

                temp_sum_centi += temp_centi;
                temp_count++;
            }
        }
    }

    if (temp_count == 0) {
        return -9999;
    }

    return (int32_t)(temp_sum_centi / temp_count);
}


void TASK_service_write_mpi_data(void *argument) {
    TASK_HELP_start_of_task();
    osDelay(5000);

    while(1) {
        int32_t last_mpi_temperature_cC = -99999;

        // If the first large buffer contains data to write
        if (MPI_buffer_one_state == MPI_MEMORY_WRITE_STATUS_AWAITING_WRITE) {
            write_mpi_data_to_memory(
                MPI_science_buffer_one,
                MPI_buffer_one_last_filled_uptime_ms
            );
            last_mpi_temperature_cC = read_avg_temperature_cC_from_mpi_data_buffer(
                MPI_science_buffer_one
            );

            MPI_buffer_one_state = MPI_MEMORY_WRITE_STATUS_READY_TO_FILL;
        }
        else if (MPI_buffer_two_state == MPI_MEMORY_WRITE_STATUS_AWAITING_WRITE) {
            write_mpi_data_to_memory(
                MPI_science_buffer_two,
                MPI_buffer_two_last_filled_uptime_ms
            );
            last_mpi_temperature_cC = read_avg_temperature_cC_from_mpi_data_buffer(
                MPI_science_buffer_two
            );

            MPI_buffer_two_state = MPI_MEMORY_WRITE_STATUS_READY_TO_FILL;
        }

        // If we have a valid averaged temperature value available:
        if (last_mpi_temperature_cC != -99999) {
            // TODO: Maybe disable this log message - it's pretty verbose.
            LOG_message(
                LOG_SYSTEM_MPI, LOG_SEVERITY_DEBUG, LOG_SINK_ALL,
                "MPI Task: Last avg temperature: %ld cC", last_mpi_temperature_cC
            );

            if ((MPI_max_temperature_shutoff_celcius > 0)
                    && ((last_mpi_temperature_cC / 100) > (int32_t)MPI_max_temperature_shutoff_celcius)
            ) {
                LOG_message(
                    LOG_SYSTEM_MPI, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
                    "MPI Task: Temperature exceeded maximum limit (%ld cC > %ld cC), stopping.",
                    last_mpi_temperature_cC, MPI_max_temperature_shutoff_celcius * 100
                );

                // Major action here - disable recording:
                MPI_disable_active_mode(MPI_REASON_FOR_STOPPING_TEMPERATURE_EXCEEDED);
            }
        }

        // Check for exceeding the start time.
        if (MPI_current_uart_rx_mode == MPI_RX_MODE_SENSING_MODE) {
            const uint32_t current_recording_duration_sec = (
                TIME_get_current_system_uptime_ms() - MPI_recording_start_uptime_ms
            ) / 1000;
            if (
                (MPI_max_recording_duration_sec > 0)
                && (current_recording_duration_sec > MPI_max_recording_duration_sec)
            ) {
                LOG_message(
                    LOG_SYSTEM_MPI, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
                    "MPI Task: Recording duration exceeded maximum limit (%lu s > %lu s), stopping.",
                    current_recording_duration_sec, MPI_max_recording_duration_sec
                );

                // Major action here - disable recording:
                MPI_disable_active_mode(MPI_REASON_FOR_STOPPING_MAX_TIME_EXCEEDED);
            }
        }

        // Do a short delay always to allow recording to start right away once enabled
        // (for the very first packet of a recording session).
        osDelay(100);
    }
}
