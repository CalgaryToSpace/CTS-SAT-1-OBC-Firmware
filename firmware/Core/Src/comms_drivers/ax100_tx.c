#include "main.h"
#include "comms_drivers/ax100_tx.h"
#include "comms_drivers/ax100_hw.h"
#include "log/log.h"
#include "debug_tools/debug_uart.h"

#include <string.h>


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
    // Construct csp header as uint32.
    uint8_t csp_header[AX100_CSP_HEADER_LENGTH_BYTES];

    // TODO: Rewrite this to do the shifting in the construction here.
    *((uint32_t *)csp_header) = (
        csp_priority | own_csp_addr | ground_station_csp_addr | ground_station_csp_port
        | own_csp_port | use_hmac | use_xtea | use_rdp | use_crc
    );

    // Put in big endian.
    destination[0] = csp_header[3];
    destination[1] = csp_header[2];
    destination[2] = csp_header[1];
    destination[3] = csp_header[0];

    // Copy data into packet.
    memcpy(destination + AX100_CSP_HEADER_LENGTH_BYTES, data, data_len);
}


/// @brief Send a csp packet over i2c to the AX100 for downlink.
/// @note Only use in this file.
static uint8_t send_bytes_to_ax100(uint8_t *packet, uint16_t packet_size) {
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



/// @brief Send data to the ax100 for downlink
/// @param data pointer to the data to downlink
/// @param data_len length of the data
/// @return 0 on success, >0 on error/failure
uint8_t AX100_downlink_bytes(uint8_t *data, uint32_t data_len) {
    if (data_len > AX100_DOWNLINK_MAX_BYTES) {
        LOG_message(
            LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_ERROR, LOG_all_sinks_except(LOG_SINK_UHF_RADIO),
            "AX100 downlink data length too long: %d > %d", data_len, AX100_DOWNLINK_MAX_BYTES
        );
        data_len = AX100_DOWNLINK_MAX_BYTES;
    }

    uint8_t packet[data_len + 4];
    prepend_csp_header(packet, data, data_len);

    // Any network layer (CSP) things should be done here (e.g., XTEA, CRC, etc.)

    // Debugging write to UART.
    DEBUG_uart_print_mixed_array(packet, data_len + 4, "AX100 Down");

    return send_bytes_to_ax100(packet, data_len + 4);
}
