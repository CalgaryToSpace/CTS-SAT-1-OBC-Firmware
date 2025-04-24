#include "main.h"
#include "comms_drivers/ax100_tx.h"
#include "log/log.h"

#include <string.h>


const uint32_t csp_priority = 3u << 30; //priority

const uint32_t own_csp_addr = 1 << 25 ; //src address
const uint32_t ground_station_csp_addr = 10 << 20; //dst address

const uint32_t ground_station_csp_port = 10 << 14; //dst port
const uint32_t own_csp_port = 10 << 8; //src port

const uint32_t use_hmac = 0 << 3; //hmac
const uint32_t use_xtea = 0 << 2; //xtea
const uint32_t use_rdp = 0 << 1; //rdp
const uint32_t use_crc = 0; //crc


const uint32_t ax100_uart_timeout_ms = 1000;

#define FEND 0xC0
#define FESC 0xDB
#define TFEND 0xDC
#define TFESC 0xDD

/// @brief Write the csp header (4 bytes) to the beginning of the packet, then copy the rest of `data` into `destination`.
/// @param destination where the assembled packet will be stored
/// @param data data to place in the packet
/// @param data_len length of data
static void prepend_csp_header(uint8_t *destination, uint8_t *data, uint32_t data_len) {
    // Construct csp header as uint32.
    uint8_t csp_header[4];

    *((uint32_t *)csp_header) = (
        csp_priority | own_csp_addr | ground_station_csp_addr | ground_station_csp_port
        | own_csp_port | use_hmac | use_xtea | use_rdp | use_crc
    );

    // put in big endian
    destination[0] = csp_header[3];
    destination[1] = csp_header[2];
    destination[2] = csp_header[1];
    destination[3] = csp_header[0];

    // Copy data into packet
    memcpy(destination + 4, data, data_len);
}

/// @brief 
/// @param data 
/// @param data_len 
/// @param result Pointer to output destination array. Must be 255 bytes long.
/// @param result_len 
static void wrap_data_in_kiss_frame(uint8_t *data, uint8_t data_len, uint8_t *result, uint8_t *result_len) {
    result[0] = 0xC0;
    result[1] = 0x00;
    uint8_t i = 2;
    while (i < data_len + 1) {
        // AX100 MTU for KISS is 255, if data is too long, truncate
        if (i >= 253) {
            LOG_message(
                LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_ERROR, LOG_SINK_ALL,
                "KISS frame too long! Data was truncated."
            );
            result[i] = 0xC0;
            *result_len = i + 1;
            return;
        }
        
        // Escape special characters.
        if (data[i] == FEND) {
            result[i] = FESC;
            result[i+1] = TFEND; 
            i += 2;
        }
        else if (data[i] == FESC) {
            result[i+0] = FESC;
            result[i+1] = TFESC;
            i += 2;
        }
        else {
            result[i+1] = data[i];
            i++;
        }
    }
    result[i] = 0xC0;
    *result_len = i+1;
}

/// @brief Sends a csp packet to the AX100 over UART.
/// @note Static. Only for use in this file.
static uint8_t send_bytes_to_ax100(uint8_t *data, uint16_t data_len) {
    if (data_len > AX100_DOWNLINK_MAX_BYTES) {
        data_len = AX100_DOWNLINK_MAX_BYTES;
    }
    uint8_t kiss_frame[255];
    uint8_t kiss_frame_len = 0;

    wrap_data_in_kiss_frame(data, data_len, kiss_frame, &kiss_frame_len);

    LOG_message(
        LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_DEBUG, LOG_all_sinks_except(LOG_SINK_UHF_RADIO),
        "Sending %d bytes to ax100.", kiss_frame_len
    );
    
    const HAL_StatusTypeDef status = HAL_UART_Transmit(
        &huart2, // FIXME: verify this is the right uart
        kiss_frame,
        kiss_frame_len, 
        ax100_uart_timeout_ms
    );

    if (status != HAL_OK) {
        LOG_message(
            LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_ERROR, LOG_all_sinks_except(LOG_SINK_UHF_RADIO),
            "HAL UART transmit error: %d", status
        );
        return status;
    }
    return 0;
}


/// @brief send data to the ax100 for downlink
/// @param data pointer to the data to downlink
/// @param data_len length of the data
/// @return 0 on success, 1 on failure
uint8_t AX100_downlink_bytes(uint8_t *data, uint32_t data_len) {
    //TODO: For now, only send as much data as can fit in one packet, we could use/implement rdp to allow downlinking of arbitrary amounts of data.
    if (data_len + 4 > AX100_DOWNLINK_MAX_BYTES) {
        // TODO: Maybe chunk into several frames instead of truncating here.
        data_len = AX100_DOWNLINK_MAX_BYTES - 4;
    }

    uint8_t packet[data_len + 4];
    prepend_csp_header(packet, data, data_len);

    // Any network layer (CSP) things should be done here (e.g., XTEA, CRC, etc.)

    return send_bytes_to_ax100(packet, data_len + 4);
}
