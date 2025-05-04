#include "self_checks/complete_self_check.h"

#include "adcs_drivers/adcs_internal_drivers.h"
#include "adcs_drivers/adcs_types.h"
#include "adcs_drivers/adcs_commands.h"
#include "comms_drivers/ax100_hw.h"
#include "gps/gps_internal_drivers.h"
#include "eps_drivers/eps_commands.h"
#include "eps_drivers/eps_channel_control.h"
#include "eps_drivers/eps_calculations.h"
#include "mpi/mpi_transceiver.h"
#include "mpi/mpi_command_handling.h"
#include "antenna_deploy_drivers/ant_commands.h"
#include "antenna_deploy_drivers/ant_internal_drivers.h"
#include "camera/camera_init.h"

#include "uart_handler/uart_handler.h"
#include "obc_temperature_sensor/obc_temperature_sensor.h"
#include "littlefs/flash_driver.h"
#include "log/log.h"
#include "debug_tools/debug_uart.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>

// TODO: Create separate self-check for the boom.

// MPI takes 3-4 seconds to boot. Wait 5 seconds to be safe.
static const uint16_t MPI_boot_duration_ms = 5000;


uint8_t CTS1_check_is_i2c_addr_alive(I2C_HandleTypeDef* hi2c, uint8_t i2c_addr) {
    const uint8_t I2C_scan_number_of_trials = 2;
    const uint32_t I2C_scan_timeout_ms = 40;

    const HAL_StatusTypeDef i2c_device_status = HAL_I2C_IsDeviceReady(
        hi2c, (i2c_addr<<1), I2C_scan_number_of_trials, I2C_scan_timeout_ms
    );

    return (i2c_device_status == HAL_OK);
}

uint8_t CTS1_check_is_adcs_alive() {
    ADCS_id_struct_t id_struct;
    const uint8_t status = ADCS_get_identification(&id_struct);
    if (status != 0) {
        LOG_message(
            LOG_SYSTEM_ADCS, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "ADCS_get_identification failed: status=%d", status
        );
        return 0;
    }
    if (id_struct.node_type != 10) {
        return 0;
    }
    if (id_struct.interface_version != 7) {
        return 0;
    }
    if (id_struct.major_firmware_version != 7) {
        return 0;
    }
    if (id_struct.minor_firmware_version != 12) {
        return 0;
    }
    return 1;
}

// uint8_t CTS1_check_is_ax100_alive() {} // TODO: Is there a way to test this?

uint8_t CTS1_check_is_gnss_responsive() {
    // Use versiona command instead of bestxyza, as bestxyza takes a variable duration, whereas
    // versiona appears to respond very quickly reliably.
    const char cmd[] = "log versiona once\n";

    // Expecting versiona may send up to about 250 bytes.
    uint8_t rx_buf[350];
    uint16_t rx_buf_received_len = 0;
    const uint8_t gps_status = GPS_send_cmd_get_response(
        cmd, strlen(cmd),
        rx_buf,
        sizeof(rx_buf),
        &rx_buf_received_len
    );

    // Clean up: Disable the GPS UART interrupt.
    GPS_set_uart_interrupt_state(0);
    HAL_Delay(20); // Allow any pending IRQs to trigger so that upcoming UART prints/logs work.

    if (gps_status != 0) {
        LOG_message(
            LOG_SYSTEM_GPS, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "GPS ERROR: Failed to send command to GPS. status=%d",
            gps_status
        );
        return 0;
    }

    // Ensure rx_buf is null-terminated.
    rx_buf[sizeof(rx_buf) - 1] = '\0';
    rx_buf[rx_buf_received_len] = '\0';
    
    LOG_message(
        LOG_SYSTEM_GPS, LOG_SEVERITY_DEBUG, LOG_SINK_ALL,
        "GPS response (%d bytes): %s",
        rx_buf_received_len,
        (char*)rx_buf
    );
    
    // Crude validation of the GNSS response:
    if (rx_buf_received_len < 5) {
        return 0;
    }

    // Validate that the response should contain the string "versiona" in it.
    if (strstr((char*)rx_buf, "#VERSIONA,") == NULL) {
        return 0;
    }
    if (strstr((char*)rx_buf, "OK") == NULL) {
        return 0;
    }
    if (strstr((char*)rx_buf, "OEM719") == NULL) {
        return 0;
    }

    return 1; // Success.
}

uint8_t CTS1_check_is_eps_responsive() {
    EPS_struct_system_status_t status;
    const uint8_t result = EPS_CMD_get_system_status(&status);

    // Note: By this point, the response is pretty validated.
    return (result == 0);
}

uint8_t CTS1_check_is_eps_thriving() {
    // Block: Check that the EPS is in "normal" mode.
    {
        EPS_struct_system_status_t status;
        const uint8_t result = EPS_CMD_get_system_status(&status);

        if (result != 0) {
            return 0;
        }

        // Check that the mode is in "normal" mode.
        if (status.mode != 1) {
            return 0;
        }
    }

    // Block: Check that the battery voltage is within the expected range.
    {
        // Check that the battery voltage is within the expected range.
        EPS_struct_pbu_housekeeping_data_eng_t pbu_data;
        const uint8_t pbu_result = EPS_CMD_get_pbu_housekeeping_data_eng(&pbu_data);

        if (pbu_result != 0) {
            return 0;
        }

        const float battery_percent = EPS_convert_battery_voltage_to_percent(
            pbu_data.battery_pack_info_each_pack[0]
        );

        if (battery_percent < 1.0 || battery_percent > 150.0) {
            return 0;
        }
    }

    return 1;
}

/// @brief Check if the MPI is dumping science data by using the lazy blocking receive method.
/// @return 1 if the MPI is dumping science data, 0 otherwise.
uint8_t CTS1_check_is_mpi_dumping() {
    MPI_set_transceiver_state(MPI_TRANSCEIVER_MODE_MISO); // OBC is listening.

    // First, cancel any ongoing DMA transfers.
    const HAL_StatusTypeDef abort_status = HAL_UART_AbortReceive(UART_mpi_port_handle);
    if (abort_status != HAL_OK) {
        LOG_message(
            LOG_SYSTEM_MPI, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "Failed to abort MPI DMA listening: HAL_Status=%d", abort_status
        );
        MPI_set_transceiver_state(MPI_TRANSCEIVER_MODE_INACTIVE);
        return 0;
    }

    // Start listening in blocking mode.
    uint8_t rx_buffer[350];
    const HAL_StatusTypeDef rx_status = HAL_UART_Receive(
        UART_mpi_port_handle,
        rx_buffer, sizeof(rx_buffer),
        400 // Timeout: 400ms is plenty.
    );
    if (rx_status != HAL_OK) {
        LOG_message(
            LOG_SYSTEM_MPI, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "Failed to receive MPI data (probably timeout). HAL_Status=%d", rx_status
        );
        MPI_set_transceiver_state(MPI_TRANSCEIVER_MODE_INACTIVE);
        return 0;
    }
    
    // Validate the response. Should contain {0x0c, 0xff, 0xff, 0x0c} (sync bytes) in it.
    uint8_t return_val = 0;
    for (uint16_t i = 0; i < sizeof(rx_buffer) - 3; i++) {
        if (
            rx_buffer[i] == 0x0c &&
            rx_buffer[i + 1] == 0xff &&
            rx_buffer[i + 2] == 0xff &&
            rx_buffer[i + 3] == 0x0c
        ) {
            return_val = 1;
            break;
        }
    }
    
    return return_val;
}

uint8_t CTS1_check_mpi_cmd_works() {
    EPS_set_channel_enabled(EPS_CHANNEL_12V_MPI, 1);
    EPS_set_channel_enabled(EPS_CHANNEL_5V_MPI, 1);
    HAL_Delay(200); // Wait for the MPI to wake up, if needed.

    // Send random command: "TC" + command_2 + scan_mode_off
    // Not certain what it does, but we power the MPI off anyway, so it gets reset anyway.
    const uint8_t cmd[] = {0x54, 0x43, 0x02, 0x00};

    uint8_t MPI_buffer[100];
    uint16_t MPI_buffer_len = 0;
    const uint8_t result = MPI_send_command_get_response(
        cmd, sizeof(cmd),
        MPI_buffer, sizeof(MPI_buffer), &MPI_buffer_len
    );

    // Clean-up: Power off the MPI.
    EPS_set_channel_enabled(EPS_CHANNEL_12V_MPI, 0);
    EPS_set_channel_enabled(EPS_CHANNEL_5V_MPI, 0);

    if (result != 0) {
        return 0;
    }

    if (MPI_buffer_len == 0) {
        return 0;
    }

    if (MPI_buffer[0] == 0x54 && MPI_buffer[1] == 0x43) {
        // The response is a "TC" response. Success.
        return 1;
    }

    return 0;
}

uint8_t CTS1_check_is_camera_responsive() {
    const uint8_t init_result = CAM_setup();
    if (init_result != 0) {
        return 0;
    }
    
    const uint8_t test_result = CAM_test();
    if (test_result != 0) {
        return 0;
    }

    // Clean-up: Power off the camera.
    const uint8_t eps_status = EPS_set_channel_enabled(EPS_CHANNEL_3V3_CAMERA, 0);
    if (eps_status != 0) {
        // Continue anyway. Just log a warning.
        LOG_message(
            LOG_SYSTEM_BOOM, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
            "Error disabling camera power channel in CTS1_check_is_camera_responsive: status=%d. Continuing.",
            eps_status
        );
        return 0;
    }

    return 1; // Success.
}

uint8_t CTS1_check_flash_alive(uint8_t flash_chip_number) {
    FLASH_deactivate_chip_select();

    const FLASH_error_enum_t result = FLASH_is_reachable(&hspi1, flash_chip_number);

    FLASH_deactivate_chip_select();
    return (result == FLASH_ERROR_NONE);
}

uint8_t CTS1_check_antenna_alive(enum ANT_i2c_bus_mcu antenna_number) {
    uint16_t raw_temperature = 0;
    const uint8_t status = ANT_CMD_measure_temp(antenna_number, &raw_temperature);
    
    if (status != 0) {
        return 0;
    }

    const int16_t temperature_cCelsius = ANT_convert_raw_temp_to_cCelsius(raw_temperature);
    return (temperature_cCelsius >= -10000) && (temperature_cCelsius <= 10000);
}


/// @brief Perform the system self-check and store the results in the provided result struct.
/// @param result Pointer to the struct to store the results of the self-check.
/// @note Powers off all systems that are tested after testing them. Powers on several systems.
void CTS1_run_system_self_check(CTS1_system_self_check_result_struct_t *result) {
    // First, clear the result struct to avoid UB if we miss any fields.
    memset(result, 0, sizeof(CTS1_system_self_check_result_struct_t));

    // Before we start powering on EPS channels, store the fault count.
    EPS_struct_pdu_overcurrent_fault_state_t fault_state_struct;
    const uint8_t EPS_fault_state_result_start = EPS_CMD_get_pdu_overcurrent_fault_state(&fault_state_struct);
    const int32_t EPS_fault_count_at_start = EPS_calculate_total_fault_count(&fault_state_struct);

    // ADCS
    result->is_adcs_i2c_addr_alive = CTS1_check_is_i2c_addr_alive(
        ADCS_i2c_HANDLE, ADCS_i2c_ADDRESS
    );
    result->is_adcs_alive = CTS1_check_is_adcs_alive();
    LOG_message(
        LOG_SYSTEM_OBC, LOG_SEVERITY_DEBUG, LOG_SINK_ALL,
        "is_adcs_i2c_addr_alive: %d, is_adcs_alive: %d",
        result->is_adcs_i2c_addr_alive,
        result->is_adcs_alive
    );

    // OBC_TEMP_SENSOR
    const int32_t obc_temperature_deg_cC = OBC_TEMP_SENSOR_get_temperature_cC();
    result->obc_temperature_works = (
        // Success condition: Between -100 and 100 degrees celsius.
        // This condition captures the known value of 99999, which is the error code.
        (obc_temperature_deg_cC >= -10000) && (obc_temperature_deg_cC <= 10000)
    );
    LOG_message(
        LOG_SYSTEM_OBC, LOG_SEVERITY_DEBUG, LOG_SINK_ALL,
        "obc_temperature_works: %d",
        result->obc_temperature_works
    );

    // AX100
    result->is_ax100_i2c_addr_alive = CTS1_check_is_i2c_addr_alive(
        AX100_I2C_HANDLE, AX100_I2C_ADDR
    );
    LOG_message(
        LOG_SYSTEM_OBC, LOG_SEVERITY_DEBUG, LOG_SINK_ALL,
        "is_ax100_i2c_addr_alive: %d",
        result->is_ax100_i2c_addr_alive
    );

    // GNSS
    result->is_gnss_responsive = CTS1_check_is_gnss_responsive();
    LOG_message(
        LOG_SYSTEM_OBC, LOG_SEVERITY_DEBUG, LOG_SINK_ALL,
        "is_gnss_responsive: %d",
        result->is_gnss_responsive
    );

    // EPS
    result->is_eps_responsive = CTS1_check_is_eps_responsive();
    result->is_eps_thriving = CTS1_check_is_eps_thriving();
    LOG_message(
        LOG_SYSTEM_OBC, LOG_SEVERITY_DEBUG, LOG_SINK_ALL,
        "is_eps_responsive: %d, is_eps_thriving: %d",
        result->is_eps_responsive,
        result->is_eps_thriving
    );

    // MPI
    EPS_set_channel_enabled(EPS_CHANNEL_12V_MPI, 1);
    EPS_set_channel_enabled(EPS_CHANNEL_5V_MPI, 1);
    HAL_Delay(MPI_boot_duration_ms); // Wait for the MPI to boot up.
    result->is_mpi_dumping = CTS1_check_is_mpi_dumping();
    result->mpi_cmd_works = CTS1_check_mpi_cmd_works();
    EPS_set_channel_enabled(EPS_CHANNEL_12V_MPI, 0);
    EPS_set_channel_enabled(EPS_CHANNEL_5V_MPI, 0);
    LOG_message(
        LOG_SYSTEM_OBC, LOG_SEVERITY_DEBUG, LOG_SINK_ALL,
        "is_mpi_dumping: %d, mpi_cmd_works: %d",
        result->is_mpi_dumping,
        result->mpi_cmd_works
    );

    // Camera
    result->is_camera_responsive = CTS1_check_is_camera_responsive();
    LOG_message(
        LOG_SYSTEM_OBC, LOG_SEVERITY_DEBUG, LOG_SINK_ALL,
        "is_camera_responsive: %d",
        result->is_camera_responsive
    );

    // Antenna
    EPS_set_channel_enabled(EPS_CHANNEL_3V3_UHF_ANTENNA_DEPLOY, 1);
    HAL_Delay(800); // 100ms not long enough. 800ms seems adequate.
    result->is_antenna_i2c_addr_a_alive = CTS1_check_is_i2c_addr_alive(&hi2c2, ANT_ADDR_A);
    result->is_antenna_i2c_addr_b_alive = CTS1_check_is_i2c_addr_alive(&hi2c3, ANT_ADDR_B);
    result->is_antenna_a_alive = CTS1_check_antenna_alive(ANT_I2C_BUS_A_MCU_A);
    result->is_antenna_b_alive = CTS1_check_antenna_alive(ANT_I2C_BUS_B_MCU_B);
    EPS_set_channel_enabled(EPS_CHANNEL_3V3_UHF_ANTENNA_DEPLOY, 0);

    // Flash
    result->flash_0_alive = CTS1_check_flash_alive(0);
    result->flash_1_alive = CTS1_check_flash_alive(1);
    result->flash_2_alive = CTS1_check_flash_alive(2);
    result->flash_3_alive = CTS1_check_flash_alive(3);
    LOG_message(
        LOG_SYSTEM_OBC, LOG_SEVERITY_DEBUG, LOG_SINK_ALL,
        "flash_alive: [%d,%d,%d,%d]",
        result->flash_0_alive,
        result->flash_1_alive,
        result->flash_2_alive,
        result->flash_3_alive
    );

    // EPS Fault Count.
    // At the end here, check the fault count again, and compare to at the start.
    const uint8_t EPS_fault_state_result_end = EPS_CMD_get_pdu_overcurrent_fault_state(&fault_state_struct);
    const int32_t EPS_fault_count_end = EPS_calculate_total_fault_count(&fault_state_struct);
    if (EPS_fault_state_result_start != 0 || EPS_fault_state_result_end != 0) {
        LOG_message(
            LOG_SYSTEM_OBC, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "Failed to get EPS fault state: %d to %d",
            EPS_fault_state_result_start,
            EPS_fault_state_result_end
        );
        result->eps_no_overcurrent_faults = 0;
    }
    else if (EPS_fault_count_at_start != EPS_fault_count_end) {
        LOG_message(
            LOG_SYSTEM_OBC, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
            "EPS fault count changed from %ld to %ld",
            EPS_fault_count_at_start,
            EPS_fault_count_end
        );
        result->eps_no_overcurrent_faults = 0;
    } else {
        result->eps_no_overcurrent_faults = 1;
    }
}

/// @brief Convert the self-check struct to a JSON string that lists the failures.
/// @param self_check_struct Self-check struct to convert to JSON.
/// @param dest_json_str Destination string to write the JSON to.
/// @param dest_json_str_size Size of the destination string (max length to write).
void CTS1_self_check_struct_TO_json_list_of_failures(
    CTS1_system_self_check_result_struct_t self_check_struct,
    char dest_json_str[], uint16_t dest_json_str_size
) {
    const char *field_names[] = {
        "obc_temperature",
        "is_adcs_i2c_addr",
        "is_adcs_alive",
        "is_ax100_i2c_addr",
        "is_gnss_responsive",
        "is_eps_responsive",
        "is_eps_thriving",
        "is_mpi_dumping_science",
        "mpi_cmd",
        "is_camera_responsive",
        "is_antenna_i2c_addr_a",
        "is_antenna_i2c_addr_b",
        "is_antenna_a_alive",
        "is_antenna_b_alive",
        "flash_0_alive",
        "flash_1_alive",
        "flash_2_alive",
        "flash_3_alive",
        "eps_no_overcurrent_faults"
    };
    
    uint8_t *field_values[] = {
        &self_check_struct.obc_temperature_works,
        &self_check_struct.is_adcs_i2c_addr_alive,
        &self_check_struct.is_adcs_alive,
        &self_check_struct.is_ax100_i2c_addr_alive,
        &self_check_struct.is_gnss_responsive,
        &self_check_struct.is_eps_responsive,
        &self_check_struct.is_eps_thriving,
        &self_check_struct.is_mpi_dumping,
        &self_check_struct.mpi_cmd_works,
        &self_check_struct.is_camera_responsive,
        &self_check_struct.is_antenna_i2c_addr_a_alive,
        &self_check_struct.is_antenna_i2c_addr_b_alive,
        &self_check_struct.is_antenna_a_alive,
        &self_check_struct.is_antenna_b_alive,
        &self_check_struct.flash_0_alive,
        &self_check_struct.flash_1_alive,
        &self_check_struct.flash_2_alive,
        &self_check_struct.flash_3_alive,
        &self_check_struct.eps_no_overcurrent_faults
    };
    
    size_t len = 0;
    len += snprintf(dest_json_str + len, dest_json_str_size - len, "{\"fail\":[");

    uint8_t is_first = 1;
    uint8_t fail_count = 0;
    uint8_t pass_count = 0;

    for (size_t i = 0; i < sizeof(field_names) / sizeof(field_names[0]); i++) {
        if (*field_values[i] == 0) {
            if (!is_first) {
                len += snprintf(dest_json_str + len, dest_json_str_size - len, ",");
            }
            len += snprintf(dest_json_str + len, dest_json_str_size - len, "\"%s\"", field_names[i]);
            is_first = 0;
            fail_count++;
        } else {
            pass_count++;
        }
    }

    len += snprintf(dest_json_str + len, dest_json_str_size - len, "],");
    len += snprintf(dest_json_str + len, dest_json_str_size - len, "\"fail_count\":%d,", fail_count);
    len += snprintf(dest_json_str + len, dest_json_str_size - len, "\"pass_count\":%d}", pass_count);

    dest_json_str[dest_json_str_size - 1] = '\0';
}
