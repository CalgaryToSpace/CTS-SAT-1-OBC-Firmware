#include "mpi/mpi_firmware_upgrade.h"

#include "log/log.h"
#include "littlefs/lfs.h"
#include "littlefs/littlefs_helper.h"
#include "debug_tools/debug_uart.h"
#include "eps_drivers/eps_channel_control.h"
#include "uart_handler/uart_handler.h"

#include "mpi/mpi_command_handling.h"
#include "mpi/mpi_transceiver.h"

static void mpi_firmware_upgrade_fail_cleanup() {
    // Set the transceiver state to OFF
    MPI_set_transceiver_state(MPI_TRANSCEIVER_MODE_INACTIVE);
}

/// @brief Upgrades the MPI firmware by reading a file from LittleFS and sending it to the MPI.
/// @details Power on the MPI, enter bootloader mode, stream the firmware file to the MPI.
/// @note Does NOT power off the MPI at the end.
/// @return 0 on success, >0 on failure.
uint8_t MPI_firmware_upgrade(const char firmware_file_path[]) {
    // Power on the MPI
    const uint8_t power_on_12v_result = EPS_set_channel_enabled(EPS_CHANNEL_12V_MPI, 1);
    const uint8_t power_on_5v_result = EPS_set_channel_enabled(EPS_CHANNEL_5V_MPI, 1);
    if (power_on_12v_result != 0 || power_on_5v_result != 0) {
        LOG_message(
            LOG_SYSTEM_MPI, LOG_SEVERITY_ERROR, LOG_all_sinks_except(LOG_SINK_FILE),
            "MPI_firmware_upgrade: Failed to power on MPI: 12V=%d, 5V=%d",
            power_on_12v_result, power_on_5v_result
        );
        mpi_firmware_upgrade_fail_cleanup();
        return 1; // Error powering on MPI
    }


    // Open the file for reading
    lfs_file_t file;
    const int32_t open_result = lfs_file_opencfg(
        &LFS_filesystem, &file, firmware_file_path, LFS_O_RDONLY, &LFS_file_cfg
    );
    if (open_result == LFS_ERR_NOENT) {
        LOG_message(
            LOG_SYSTEM_MPI, LOG_SEVERITY_ERROR, LOG_all_sinks_except(LOG_SINK_FILE),
            "MPI_firmware_upgrade: File not found"
        );
        mpi_firmware_upgrade_fail_cleanup();
        return 2;
    }
    else if (open_result < 0) {
        LOG_message(
            LOG_SYSTEM_MPI, LOG_SEVERITY_ERROR, LOG_all_sinks_except(LOG_SINK_FILE),
            "MPI_firmware_upgrade: Failed to open file: %ld", open_result
        );
        mpi_firmware_upgrade_fail_cleanup();
        return 3;
    }

    // Prep the transceiver for firmware upgrade
    MPI_set_transceiver_state(MPI_TRANSCEIVER_MODE_MOSI);

    // Enter bootloader mode
    const uint8_t bootloader_command[] = {0x54, 0x43, MPI_COMMAND_CODE_BL_ENTER_BOOTLOADER}; // "TC" + command_1 + bootloader mode
    const HAL_StatusTypeDef tx_result = HAL_UART_Transmit(
        UART_mpi_port_handle, (uint8_t*)bootloader_command, sizeof(bootloader_command), 1000
    );
    if (tx_result != HAL_OK) {
        LOG_message(
            LOG_SYSTEM_MPI, LOG_SEVERITY_ERROR, LOG_all_sinks_except(LOG_SINK_FILE),
            "MPI_firmware_upgrade: Failed to send bootloader command: %d", tx_result
        );
        lfs_file_close(&LFS_filesystem, &file);
        mpi_firmware_upgrade_fail_cleanup();
        return 4; // Error sending bootloader command
    }

    // Read the firmware data
    uint8_t buffer[2048];
    int32_t bytes_read;
    uint32_t total_bytes_sent = 0;
    while ((bytes_read = lfs_file_read(&LFS_filesystem, &file, buffer, sizeof(buffer))) > 0) {
        // Send the read data to the MPI
        const HAL_StatusTypeDef send_result = HAL_UART_Transmit(
            UART_mpi_port_handle, buffer, bytes_read, 1000
        );
        if (send_result != 0) {
            LOG_message(
                LOG_SYSTEM_MPI, LOG_SEVERITY_ERROR, LOG_all_sinks_except(LOG_SINK_FILE),
                "MPI_firmware_upgrade: Failed to send data to MPI (HAL_Err=%d)", send_result
            );
            lfs_file_close(&LFS_filesystem, &file);
            mpi_firmware_upgrade_fail_cleanup();
            return 5;
        }
        total_bytes_sent += bytes_read;

        DEBUG_uart_print_str("."); // Print a dot for each chunk sent, for progress indication.
    }

    LOG_message(
        LOG_SYSTEM_MPI, LOG_SEVERITY_NORMAL, LOG_all_sinks_except(LOG_SINK_FILE),
        "MPI_firmware_upgrade: Firmware upgrade executed. Sent %ld bytes.",
        total_bytes_sent
    );

    // Close the file
    const int32_t close_result = lfs_file_close(&LFS_filesystem, &file);
    if (close_result < 0) {
        LOG_message(
            LOG_SYSTEM_MPI, LOG_SEVERITY_ERROR, LOG_all_sinks_except(LOG_SINK_FILE),
            "MPI_firmware_upgrade: Failed to close file: %ld", close_result
        );
        mpi_firmware_upgrade_fail_cleanup();
        return 6; // Error closing file
    }

    // Nominal completion.
    mpi_firmware_upgrade_fail_cleanup();

    return 0; // Success
}