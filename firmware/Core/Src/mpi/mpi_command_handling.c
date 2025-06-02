#include "mpi/mpi_command_handling.h"
#include "mpi/mpi_types.h"
#include "mpi/mpi_transceiver.h"
#include "eps_drivers/eps_channel_control.h"
#include "uart_handler/uart_handler.h"
#include "log/log.h"
#include "debug_tools/debug_uart.h"
#include "littlefs/littlefs_helper.h"
#include "timekeeping/timekeeping.h"
#include "transforms/arrays.h"

#include "main.h"
#include "stm32l4xx_hal.h"

#include <string.h>
#include <stdio.h>

/// @brief Timeout duration for transmit HAL call, in milliseconds.
static const uint16_t MPI_TX_TIMEOUT_DURATION_MS = 100;

/// @brief Timeout duration for receive in milliseconds. Same between bytes and at the start.
static const uint16_t MPI_RX_TIMEOUT_DURATION_MS = 2000;

volatile MPI_rx_mode_t MPI_current_uart_rx_mode = MPI_RX_MODE_NOT_LISTENING_TO_MPI;


uint8_t MPI_science_data_file_is_open = 0;
uint32_t MPI_science_data_bytes_lost = 0;
lfs_file_t MPI_science_data_file_pointer;
uint32_t MPI_recording_start_uptime_ms;


/// @brief Sends commandcode+params to the MPI as bytes
/// @param bytes_to_send Buffer containing the telecommand + params (IF ANY) as hex bytes
/// @param bytes_to_send_len Size of telecommand buffer
/// @param rx_buffer Buffer to store incoming response from the MPI
/// @param rx_buffer_max_size The maximum size of the MPI response buffer
/// @param rx_buffer_len Pointer to variable that will contain the length of the populated MPI response buffer 
/// @return 0: Success, 2: Failed UART transmission, 3: Failed UART reception, 
///         4: Timeout waiting for 1st byte from MPI, 8: Not enough space in the MPI response buffer
/// @note If the MPI is in "science data" mode, it will be disabled after the command is executed.
uint8_t MPI_send_command_get_response(
    const uint8_t *bytes_to_send, const size_t bytes_to_send_len, 
    uint8_t *rx_buffer, const size_t rx_buffer_max_size, 
    uint16_t *rx_buffer_len
) {
    if (rx_buffer_max_size < (bytes_to_send_len + 1)) {
        return 8; // Not enough space in the MPI response buffer
    }

    // Stop any ongoing DMA reception
    HAL_UART_DMAStop(UART_mpi_port_handle);
    HAL_UART_AbortReceive(UART_mpi_port_handle);

    // Clear volatile buffer manually
    for (uint16_t i = 0; i < UART_mpi_buffer_len; i++) {
        UART_mpi_buffer[i] = 0;
    }

    MPI_set_transceiver_state(MPI_TRANSCEIVER_MODE_DUPLEX);
    MPI_current_uart_rx_mode = MPI_RX_MODE_COMMAND_MODE;
    HAL_Delay(50);  // Stabilization delay

    // Transmit command
    const HAL_StatusTypeDef tx_status = HAL_UART_Transmit(
        UART_mpi_port_handle, bytes_to_send, bytes_to_send_len, MPI_TX_TIMEOUT_DURATION_MS);
    if (tx_status != HAL_OK) {
        MPI_current_uart_rx_mode = MPI_RX_MODE_NOT_LISTENING_TO_MPI;
        return 2; // UART TX failure
    }

    // Blocking reception starts here
    uint16_t received_len = 0;

    while (received_len < rx_buffer_max_size) {
        uint8_t byte = 0;
        const HAL_StatusTypeDef rx_status = HAL_UART_Receive(
            UART_mpi_port_handle, &byte, 1, MPI_RX_TIMEOUT_DURATION_MS);

        if (rx_status == HAL_OK) {
            rx_buffer[received_len++] = byte;
        } else if (rx_status == HAL_TIMEOUT) {
            if (received_len == 0) {
                // Timeout waiting for first byte
                LOG_message(
                    LOG_SYSTEM_MPI, LOG_SEVERITY_DEBUG, LOG_SINK_ALL,
                    "Timeout waiting for 1st byte. received_len=%u", received_len);
                DEBUG_uart_print_str("rx_buffer[0:10]: ");
                DEBUG_uart_print_array_hex(rx_buffer, 10);
                DEBUG_uart_print_str("\n");

                MPI_set_transceiver_state(MPI_TRANSCEIVER_MODE_INACTIVE);
                MPI_current_uart_rx_mode = MPI_RX_MODE_NOT_LISTENING_TO_MPI;
                return 4;
            } else {
                // No more bytes coming after partial reception
                break;
            }
        } else {
            // UART error
            MPI_set_transceiver_state(MPI_TRANSCEIVER_MODE_INACTIVE);
            MPI_current_uart_rx_mode = MPI_RX_MODE_NOT_LISTENING_TO_MPI;
            return 3;
        }
    }

    *rx_buffer_len = received_len;

    LOG_message(
        LOG_SYSTEM_MPI, LOG_SEVERITY_DEBUG, LOG_SINK_ALL,
        "Blocking MPI response received: rx_buffer_len=%u", *rx_buffer_len
    );

    MPI_set_transceiver_state(MPI_TRANSCEIVER_MODE_INACTIVE);
    MPI_current_uart_rx_mode = MPI_RX_MODE_NOT_LISTENING_TO_MPI;

    return 0;  // Success
}


/// @brief The MPI responds to each telecommand with a response code consisting of an echo of the 
///        telecommand code byte and a status byte (either 254 or OxFE) for success or error code otherwise.
/// @param tx_buffer MPI telecommand buffer containing bytes sent
/// @param rx_buffer MPI response buffer containing bytes received
/// @param tx_buffer_size Size of the MPI response buffer
/// @return 0: MPI successfully executed telecommand, 5: MPI failed to execute telecommand, 
///         6: Invalid response from the MPI
uint8_t MPI_validate_command_response(
    const uint8_t command_code, uint8_t *rx_buffer, 
    const uint16_t rx_buffer_len
) {  
    // Ensure enough bytes were received  
    if (rx_buffer_len < 2) {
        return 17; // Error code: MPI rx buffer too small
    }

    // Verify if the MPI response echos the cmd sent
    if (command_code != rx_buffer[0]) {
        return 16; // Error code: Invalid response from the MPI
    }

    // Verify if the MPI response responds with a success byte
    const uint8_t command_status = rx_buffer[1];
    if (command_status != MPI_COMMAND_SUCCESS_RESPONSE_VALUE) {
        return 15; // Error code: MPI failed to execute command
    }

    return 0; //  MPI executed the cmd successfully
}

/// @brief Turns on MPI by setting the corresponding EPS channel.
static void MPI_power_on() {
    // Power On MPI 5v
    const uint8_t mpi_5v_result = EPS_set_channel_enabled(EPS_CHANNEL_5V_MPI, 1);
    if (mpi_5v_result != 0) {
        LOG_message(LOG_SYSTEM_MPI, LOG_SEVERITY_WARNING, LOG_SINK_ALL, 
            "MPI 5v could not be powered on (EPS_set_channel_enabled->%d)",
            mpi_5v_result
        );
    }

    // Power On MPI 12v
    const uint8_t mpi_12v_result = EPS_set_channel_enabled(EPS_CHANNEL_12V_MPI, 1);
    if (mpi_12v_result != 0) {
        LOG_message(LOG_SYSTEM_MPI, LOG_SEVERITY_WARNING, LOG_SINK_ALL, 
            "MPI 12v could not be powered on (EPS_set_channel_enabled->%d)",
            mpi_12v_result
        );
    }
}

static int8_t MPI_write_file_header() {
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

    char header_str[200];
    snprintf(
        header_str, sizeof(header_str),
        "{\"mpi_start\":1,\"uptime_ms\":%ld,\"timestamp\":\"%s\",\"datetime\":\"%s\",\"timestamp_ms\":%s}",
        uptime_ms,
        timestamp_fmt_str,
        datetime_fmt_str,
        timestamp_ms_str
    );

    const lfs_ssize_t write_timestamp_result = lfs_file_write(
        &LFS_filesystem, &MPI_science_data_file_pointer,
        header_str, strlen(header_str)
    );
    if (write_timestamp_result < 0) {
        LOG_message(
            LOG_SYSTEM_MPI, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
            "MPI Header: Error writing timestamp to file: %ld", write_timestamp_result
        );
        return write_timestamp_result;
    }

    LOG_message(
        LOG_SYSTEM_MPI, LOG_SEVERITY_NORMAL, LOG_all_sinks_except(LOG_SINK_FILE),
        "MPI Header: File header written successfully"
    );
    return 0; // Success.
}

/// @brief Turns on MPI and prepares a LFS file to store MPI data in.
/// @return 0: System successfully prepared for MPI data, < 0: Error
static int8_t MPI_prepare_receive_data(const char output_file_path[]) {
    MPI_power_on();

    // Start the timer to track time past since we powered on MPI
    const uint32_t start_time = HAL_GetTick();

    // Mount LFS if not already mounted
    if (!LFS_is_lfs_mounted) {
        LFS_mount();
    }

    // If the old file is open, close the file
    if (MPI_science_data_file_is_open == 1) {
        const int8_t close_result = lfs_file_close(&LFS_filesystem, &MPI_science_data_file_pointer);
        if (close_result < 0) {
            LOG_message(
                LOG_SYSTEM_MPI, LOG_SEVERITY_WARNING, LOG_all_sinks_except(LOG_SINK_FILE),
                "Error closing old file: %d", close_result
            );
            return close_result;
        }
        LOG_message(
            LOG_SYSTEM_MPI, LOG_SEVERITY_WARNING, LOG_all_sinks_except(LOG_SINK_FILE),
            "Old File successfully closed"
        );
        MPI_science_data_file_is_open = 0;
    }

    // Open / Create the file
    const int8_t open_result = lfs_file_opencfg(
        &LFS_filesystem, &MPI_science_data_file_pointer,
        output_file_path,
        LFS_O_WRONLY | LFS_O_CREAT | LFS_O_APPEND, &LFS_file_cfg
    );
    
    // Check if open successful
    if (open_result < 0) {
        LOG_message(
            LOG_SYSTEM_MPI, LOG_SEVERITY_WARNING, LOG_all_sinks_except(LOG_SINK_FILE),
            "Error opening / creating file: %s", output_file_path
        );
        return open_result;
    }

    // Change the state to state file is open
    MPI_science_data_file_is_open = 1;
    LOG_message(
        LOG_SYSTEM_MPI, LOG_SEVERITY_NORMAL, LOG_all_sinks_except(LOG_SINK_FILE),
        "Opened/created file: %s", output_file_path
    );

    const int8_t header_status = MPI_write_file_header();
    if (header_status < 0) {
        LOG_message(
            LOG_SYSTEM_MPI, LOG_SEVERITY_WARNING, LOG_all_sinks_except(LOG_SINK_FILE),
            "Error writing file header: %d", header_status
        );
        return header_status; // Error writing file header
    }
    
    // Total 5 second delay to make sure MPI is booted
    const int32_t power_on_delay = (5000 - (HAL_GetTick() - start_time));
    if (power_on_delay > 0) {
        HAL_Delay(power_on_delay);
    }
    
    return 0;
}


/// @brief Turns on MPI science mode and Enables DMA interrupt for MPI channel.
/// @return 0: MPI and DMA successfully enabled, < 0: Error
uint8_t MPI_enable_active_mode(const char output_file_path[]) {
    // Turn on the MPI and setup LFS
    const uint8_t prepare_result = MPI_prepare_receive_data(output_file_path);
    if (prepare_result != 0) {
        LOG_message(LOG_SYSTEM_MPI, LOG_SEVERITY_ERROR, LOG_SINK_ALL, 
            "MPI could not be powered on (MPI_prepare_receive_data err: %d)", prepare_result);
        return 5;
    }

    // Abort any previous reception.
    // Very important. Makes everything work.
    HAL_UART_AbortReceive(UART_mpi_port_handle);

    MPI_set_transceiver_state(MPI_TRANSCEIVER_MODE_MOSI); // Set the MPI transceiver to MOSI mode
    HAL_Delay(50);

    // Send command to start MPI science data.
    const uint8_t tx_buffer[3] = {0x54, 0x43, 0x13}; // Start data command (0x13 = d19 = START)
    const HAL_StatusTypeDef tx_result = HAL_UART_Transmit(
        UART_mpi_port_handle,
        tx_buffer, sizeof(tx_buffer),
        MPI_TX_TIMEOUT_DURATION_MS
    );
    if (tx_result != HAL_OK) {
        LOG_message(
            LOG_SYSTEM_MPI, LOG_SEVERITY_ERROR, LOG_SINK_ALL, 
            "MPI HAL_UART_Transmit error (HAL_UART_Transmit result: %d)", tx_result
        );
        return 4;
    }

    MPI_set_transceiver_state(MPI_TRANSCEIVER_MODE_MISO); // Set the MPI transceiver to MISO mode (better power efficiency than DUPLEX).
    MPI_current_uart_rx_mode = MPI_RX_MODE_SENSING_MODE;
    
    // Init counters, etc.
    MPI_science_data_bytes_lost = 0;
    MPI_recording_start_uptime_ms = HAL_GetTick();


    // Receive MPI response actively with 8192 buffer size.
    const HAL_StatusTypeDef rx_status = HAL_UART_Receive_DMA(
        UART_mpi_port_handle, (uint8_t*) UART_mpi_rx_dma_buffer, UART_mpi_rx_dma_buffer_len);

    if (rx_status != HAL_OK) {
        // Note: Saksham's original code didn't call HAL_UART_DMAStop here if HAL_BUSY.
        HAL_UART_DMAStop(UART_mpi_port_handle);

        LOG_message(
            LOG_SYSTEM_MPI, LOG_SEVERITY_ERROR, LOG_SINK_ALL, 
            "MPI HAL_UART_Receive_DMA error (HAL_UART_Receive_DMA result: %d)", rx_status
        );

        return 3; // Error code: Failed to start UART reception
    }

    return 0;
}

/// @brief Turns off MPI by resetting the corresponding EPS channel.
static void MPI_power_off() {
    // Power off the MPI 5v
    const uint8_t mpi_5v_result = EPS_set_channel_enabled(EPS_CHANNEL_5V_MPI, 0);
    if (mpi_5v_result != 0) {
        LOG_message(LOG_SYSTEM_MPI, LOG_SEVERITY_WARNING, LOG_SINK_ALL, 
            "MPI 5v could not be powered off (EPS_set_channel_enabled->%d)",
            mpi_5v_result
        );
    }
    // Power off the MPI 12v
    const uint8_t mpi_12v_result = EPS_set_channel_enabled(EPS_CHANNEL_12V_MPI, 0);
    if (mpi_12v_result != 0) {
        LOG_message(LOG_SYSTEM_MPI, LOG_SEVERITY_WARNING, LOG_SINK_ALL, 
            "MPI 12v could not be powered off (EPS_set_channel_enabled->%d)",
            mpi_12v_result
        );
    }
}

uint8_t MPI_disable_active_mode() {
    MPI_power_off();

    // Set the MPI State to not handle any receiving data
    MPI_set_transceiver_state(MPI_TRANSCEIVER_MODE_INACTIVE); // Set the MPI transceiver to inactive
    const HAL_StatusTypeDef stop_status = HAL_UART_DMAStop(UART_mpi_port_handle);
    if (stop_status != HAL_OK) {
        LOG_message(
            LOG_SYSTEM_MPI, LOG_SEVERITY_ERROR, LOG_SINK_ALL, 
            "MPI HAL_UART_DMAStop error (HAL_UART_DMAStop result: %d)", stop_status
        );
        // Steamroll here. Still want to close the file.
    }

    if (MPI_current_uart_rx_mode != MPI_RX_MODE_SENSING_MODE) {
        LOG_message(
            LOG_SYSTEM_MPI, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
            "MPI stop command called when not currently in sensing mode. Can't close file."
        );
        MPI_current_uart_rx_mode = MPI_RX_MODE_NOT_LISTENING_TO_MPI; // Reset the state.
        return 1;
    }

    MPI_current_uart_rx_mode = MPI_RX_MODE_NOT_LISTENING_TO_MPI; // Set UART mode to not listening.

    // Close the file. The file in storage is not updated until the file is closed successfully.
    const int8_t close_result = lfs_file_close(&LFS_filesystem, &MPI_science_data_file_pointer);
    MPI_science_data_file_is_open = 0;
    if (close_result < 0) {
        LOG_message(
            LOG_SYSTEM_MPI, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "MPI stop: Error closing file: %d", close_result
        );
        return 2;
    }

    // Log that file has been successfully closed 
    LOG_message(
        LOG_SYSTEM_MPI, LOG_SEVERITY_DEBUG, LOG_SINK_ALL,
        "MPI stop: File closed successfully"
    );

    // Get the file size before closing the file
    const lfs_ssize_t file_size = lfs_file_size(&LFS_filesystem, &MPI_science_data_file_pointer);
    if (file_size < 0) {
        LOG_message(
            LOG_SYSTEM_MPI, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "MPI stop: Error getting file size: %ld", file_size
        );
    }
    
    // Log MPI science data stats  
    LOG_message(
        LOG_SYSTEM_MPI, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
        "{\"data_stored_bytes\": %ld, \"data_lost_bytes\": %lu, \"time_taken_ms\": %lu }",
        file_size,
        MPI_science_data_bytes_lost,
        HAL_GetTick() - MPI_recording_start_uptime_ms 
    );

    return 0;
}