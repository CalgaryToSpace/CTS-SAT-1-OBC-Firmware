#include "self_checks/complete_self_check.h"

#include "adcs_drivers/adcs_internal_drivers.h"
#include "adcs_drivers/adcs_types.h"
#include "adcs_drivers/adcs_commands.h"
#include "gps/gps_internal_drivers.h"
#include "eps_drivers/eps_commands.h"
#include "eps_drivers/eps_channel_control.h"
#include "eps_drivers/eps_calculations.h"
#include "mpi/mpi_transceiver.h"
#include "mpi/mpi_command_handling.h"
#include "antenna_deploy_drivers/ant_commands.h"
#include "antenna_deploy_drivers/ant_internal_drivers.h"

#include "uart_handler/uart_handler.h"
#include "obc_temperature_sensor/obc_temperature_sensor.h"
#include "littlefs/flash_driver.h"
#include "log/log.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>

// TODO: Create separate self-check for the boom.


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

// uint8_t CTS1_check_is_ax100_alive(); // TODO: Is there a way to test this?
uint8_t CTS1_check_is_gnss_responsive() {
    const char cmd[] = "DEAR GPS, PLEASE RESPOND\n"; // FIXME: What to send here?

    uint8_t rx_buf[100];
    uint16_t rx_buf_received_len = 0;
    const uint8_t gps_status = GPS_send_cmd_get_response(
        cmd, strlen(cmd),
        rx_buf, sizeof(rx_buf),
        &rx_buf_received_len
    );

    if (gps_status != 0) {
        GPS_set_uart_interrupt_state(0); // In case: Disable the GPS UART interrupt.
        return 0;
    }

    // FIXME: Validate the response in the `rx_buf` (and also the length of it in `rx_buf_received_len`)

    GPS_set_uart_interrupt_state(0); // In case: Disable the GPS UART interrupt.
    return 0;
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

uint8_t CTS1_check_is_mpi_dumping() {
    EPS_set_channel_enabled(EPS_CHANNEL_12V_MPI, 1);
    EPS_set_channel_enabled(EPS_CHANNEL_5V_MPI, 1);
    MPI_set_transceiver_state(MPI_TRANSCEIVER_MODE_MISO); // OBC is listening.

    MPI_current_uart_rx_mode = MPI_RX_MODE_COMMAND_MODE;
    UART_mpi_buffer_write_idx = 0;

    HAL_Delay(200); // Wait for the MPI to send some stuff.

    // Check that the MPI has sent some stuff.
    const uint8_t return_val = (UART_mpi_last_write_time_ms >= (HAL_GetTick() - 100));
        
    EPS_set_channel_enabled(EPS_CHANNEL_12V_MPI, 0);
    EPS_set_channel_enabled(EPS_CHANNEL_5V_MPI, 0);
    MPI_set_transceiver_state(MPI_TRANSCEIVER_MODE_INACTIVE);
    MPI_current_uart_rx_mode = MPI_RX_MODE_NOT_LISTENING_TO_MPI;
    HAL_UART_DMAStop(UART_mpi_port_handle);
    
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
    EPS_set_channel_enabled(EPS_CHANNEL_3V3_CAMERA, 1);
    HAL_Delay(200); // Wait for the camera to power up.

    const uint8_t number_of_bytes_to_receive = 16;

    // Clear the `UART_camera_buffer` before receiving the response.
    for (uint8_t i = 0; i < number_of_bytes_to_receive; i++) {
        UART_camera_buffer[i] = 0;
    }

    HAL_UART_Receive_DMA(
        UART_camera_port_handle, (uint8_t*)&UART_camera_buffer, number_of_bytes_to_receive
    );

    // Send the "request a test string without flash" command.
    const char cmd[] = "t";
    HAL_UART_Transmit(UART_camera_port_handle, (uint8_t*)cmd, strlen(cmd), 100);

    const uint32_t start_time = HAL_GetTick();
    uint8_t response_received = 0;
    while (HAL_GetTick() - start_time < 800) {
        if (UART_camera_last_write_time_ms >= (HAL_GetTick() - 100)) {
            response_received = 1;
            break;
        }
    }

    // Cleanup actions.
    EPS_set_channel_enabled(EPS_CHANNEL_3V3_CAMERA, 0);

    if (response_received == 0) {
        return 0;
    }

    // Validate the response - all chars should be '@', 0-9, A-F, '\n', '\r'.
    uint8_t valid_response = 1;
    for (uint8_t i = 0; i < 16; i++) {
        const uint8_t c = UART_camera_buffer[i];
        if (
            (c < '0' || c > '9') &&
            (c < 'A' || c > 'F') &&
            c != '@' &&
            c != '\n' &&
            c != '\r'
        ) {
            valid_response = 0;
            break;
        }
    }
    HAL_UART_DMAStop(UART_camera_port_handle);

    return valid_response;
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
        ADCS_i2c_HANDLE, // Uses the same I2C port.
        0x05 // TODO: Set to the #define somewhere.
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
    result->is_mpi_dumping = CTS1_check_is_mpi_dumping();
    result->mpi_cmd_works = CTS1_check_mpi_cmd_works();
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
    HAL_Delay(100);
    result->is_antenna_i2c_addr_a_alive = CTS1_check_is_i2c_addr_alive(&hi2c2, ANT_ADDR_A);
    result->is_antenna_i2c_addr_b_alive = CTS1_check_is_i2c_addr_alive(&hi2c2, ANT_ADDR_B);
    result->is_antenna_a_alive = CTS1_check_antenna_alive(ANT_I2C_BUS_A_MCU_A);
    result->is_antenna_b_alive = CTS1_check_antenna_alive(ANT_I2C_BUS_B_MCU_B);
    EPS_set_channel_enabled(EPS_CHANNEL_3V3_UHF_ANTENNA_DEPLOY, 0);
    LOG_message(
        LOG_SYSTEM_OBC, LOG_SEVERITY_DEBUG, LOG_SINK_ALL,
        "is_antenna_i2c_addr_a_alive: %d, is_antenna_i2c_addr_b_alive: %d, is_antenna_a_alive: %d, is_antenna_b_alive: %d",
        result->is_antenna_i2c_addr_a_alive,
        result->is_antenna_i2c_addr_b_alive,
        result->is_antenna_a_alive,
        result->is_antenna_b_alive
    );

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
}


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
        "flash_3_alive"
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
        &self_check_struct.flash_3_alive
    };
    
    char buffer[dest_json_str_size];
    size_t len = 0;
    len += snprintf(buffer + len, dest_json_str_size - len, "{\"fail\":[");
    
    int first = 1;
    for (size_t i = 0; i < sizeof(field_names) / sizeof(field_names[0]); i++) {
        if (*field_values[i] == 0) {
            if (!first) {
                len += snprintf(buffer + len, dest_json_str_size - len, ",");
            }
            len += snprintf(buffer + len, dest_json_str_size - len, "\"%s\"", field_names[i]);
            first = 0;
        }
    }
    
    len += snprintf(buffer + len, dest_json_str_size - len, "]}");
    
    strncpy(dest_json_str, buffer, dest_json_str_size - 1);
    dest_json_str[dest_json_str_size - 1] = '\0';
}
