#include "main.h"
#include "comms_drivers/ax100_tx.h"
#include "comms_drivers/ax100_hw.h"
#include "log/log.h"
#include "debug_tools/debug_uart.h"
#include "rtos_tasks/rtos_bootup_operation_fsm_task.h"
#include "comms_drivers/rf_antenna_switch.h"

#include <string.h>
#include <stdint.h>

/// @brief When enabled, the radio packets will be sent to the Debug UART for debugging purposes.
/// @note Only useful for configuration in ground testing.
uint32_t AX100_enable_downlink_uart_logs = 0;

/// @brief When enabled, log messages like "Downlink inhibited: ..." will be sent to the Debug UART.
/// @note Logs are important to ensure that radio downlinks aren't sent before the antenna is deployed.
///       These logs get very annoying though, so they can be turned off with this variable.
/// @note Only useful for configuration in ground testing.
uint32_t AX100_enable_downlink_inhibited_uart_logs = 1;

static const uint32_t csp_priority = 3u << 30; // priority

static const uint32_t own_csp_addr = 1u << 25 ; // src address
static const uint32_t ground_station_csp_addr = 10u << 20; // dst address

static const uint32_t ground_station_csp_port = 10u << 14; // dst port
static const uint32_t own_csp_port = 10u << 8; // src port

static const uint32_t use_hmac = 0 << 3; // hmac
static const uint32_t use_xtea = 0 << 2; // xtea
static const uint32_t use_rdp = 0 << 1; // rdp
static const uint32_t use_crc = 0; // crc


static const uint32_t AX100_i2c_tx_timeout_ms = 100;


/// @brief Write the csp header (4 bytes) to the beginning of the packet, then copy the rest of `data` into `destination`.
/// @param destination where the assembled packet will be stored
/// @param data data to place in the packet
/// @param data_len length of data
static void prepend_csp_header(uint8_t *destination, uint8_t *data, uint32_t data_len) {
    // TODO: Rewrite this to do the shifting in the construction here.
    
    const uint32_t csp_header = (
        csp_priority | own_csp_addr | ground_station_csp_addr |
        ground_station_csp_port | own_csp_port | use_hmac | use_xtea | use_rdp | use_crc
    );

    destination[0] = (csp_header >> 24) & 0xFF;
    destination[1] = (csp_header >> 16) & 0xFF;
    destination[2] = (csp_header >> 8) & 0xFF;
    destination[3] = csp_header & 0xFF;

    // Copy data into packet.
    memcpy(&destination[AX100_CSP_HEADER_LENGTH_BYTES], data, data_len);
}


/// @brief Send a csp packet over i2c to the AX100 for downlink.
/// @note Only use in this file.
static uint8_t send_bytes_to_ax100(uint8_t *packet, uint16_t packet_size) {
    if (CTS1_operation_state != CTS1_OPERATION_STATE_NOMINAL_WITH_RADIO_TX) {
        // Recall: Do not transmit on the AX100 until the antenna is deployed.
        // The Bootup Operation FSM task will set the operation mode to NOMINAL_WITH_RADIO_TX
        // when the antenna is deployed.
        if (AX100_enable_downlink_inhibited_uart_logs) {
            DEBUG_uart_print_str("AX100 downlink inhibited because antenna not deployed.\n");
        }
        
        // Return success to avoid lots of errors. It's not really an error case, as this is expected during early ops.
        return 0;
    }

    const HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(
        AX100_I2C_HANDLE,
        AX100_I2C_ADDR << 1, 
        packet,
        packet_size, 
        AX100_i2c_tx_timeout_ms
    );
    
    if (status != HAL_OK) {
        LOG_message(
            LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_ERROR, LOG_all_sinks_except(LOG_SINK_UHF_RADIO),
            "HAL I2C transmit error: %d", status
        );
        return status;
    }
    
    return 0;
}

// Statically allocated packet buffer for downlink.
// Note: Only used in `AX100_downlink_bytes`
static uint8_t packet_buffer_including_csp_header[AX100_DOWNLINK_MAX_BYTES + AX100_CSP_HEADER_LENGTH_BYTES];

/// @brief Send data to the ax100 for downlink
/// @param data pointer to the data to downlink
/// @param data_len length of the data
/// @return 0 on success, >0 on error/failure
uint8_t AX100_downlink_bytes(uint8_t *data, uint16_t data_len) {
    if (data_len > AX100_DOWNLINK_MAX_BYTES) {
        LOG_message(
            LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_WARNING, LOG_all_sinks_except(LOG_SINK_UHF_RADIO),
            "AX100 downlink data length too long: %d > %d", data_len, AX100_DOWNLINK_MAX_BYTES
        );
        data_len = AX100_DOWNLINK_MAX_BYTES;
    }

    prepend_csp_header(packet_buffer_including_csp_header, data, data_len);

    // Any network layer (CSP) things should be done here (e.g., XTEA, CRC, etc.)

    // Debugging write to UART.
    if (AX100_enable_downlink_uart_logs) {
        DEBUG_uart_print_mixed_array(
            packet_buffer_including_csp_header, data_len + AX100_CSP_HEADER_LENGTH_BYTES,
            ((COMMS_active_rf_switch_antenna == 1) ? "AX100 Down [ANT1]" : "AX100 Down [ANT2]")
        );
    }

    return send_bytes_to_ax100(packet_buffer_including_csp_header, data_len + AX100_CSP_HEADER_LENGTH_BYTES);
}
