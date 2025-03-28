#include "main.h"
#include "stm32l4xx_hal_i2c.h"
#include "stm32l4xx_hal_def.h"
#include "cts_comms_tx.h"
#include "log/log.h"
#include "memory.h"

const uint32_t csp_priority = 3u << 30; //priority

const uint32_t own_csp_addr = 1 << 25 ; //src address
const uint32_t ground_station_csp_addr = 10 << 20; //dst address

const uint32_t ground_station_csp_port = 10 << 14; //dst port
const uint32_t own_csp_port = 10 << 8; //src port

const uint32_t use_hmac = 0 << 3; //hmac
const uint32_t use_xtea = 0 << 2; //xtea
const uint32_t use_rdp = 0 << 1; //rdp
const uint32_t use_crc = 0; //crc


#define CSP_MAX_TX_SIZE 220 //TODO: figure out correct value
const uint8_t ax100_i2c_addr = 0x05 << 1;
const uint32_t ax100_i2c_timeout_ms = 1000;



/// @brief perpend the csp header to the data
/// @param result where the assembled packet will be stored
/// @param data data to place in the packet
/// @param data_len length of data
static void perpend_csp_header(uint8_t *result, uint8_t *data, uint32_t data_len) {
    //construct csp header as uint32
    uint8_t csp_header[4];
      *((uint32_t *)csp_header) = ( csp_priority | own_csp_addr | ground_station_csp_addr | ground_station_csp_port | own_csp_port | use_hmac | use_xtea | use_rdp | use_crc);    

    // put in big endian
    result[0] = csp_header[3];
    result[1] = csp_header[2];
    result[2] = csp_header[1];
    result[3] = csp_header[0];

    //copy data into packet
    memcpy(result + 4, data, data_len);
}

/// @brief only for use in this file, sends a csp packet over i2c 
static uint8_t send_csp_packet_over_i2c(uint8_t *packet, uint16_t packet_size) {
    HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(
        &hi2c1, 
        ax100_i2c_addr, 
        packet,
        packet_size, 
        ax100_i2c_timeout_ms
    );
    if (status != HAL_OK) {
        LOG_message(
            LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_ERROR, LOG_all_sinks_except(LOG_SINK_UHF_RADIO),
            "HAL I2C transmit error: %d", status
        );
        return status;
    }
    for (int i = 0; i < packet_size; i++) {
        LOG_message(
            LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_DEBUG, LOG_all_sinks_except(LOG_SINK_UHF_RADIO),
            " %x ", packet[i]
        );
    }
    return 0;
}


/// @brief downlinks data to the ground station.
/// @param data pointer to the data to downlink
/// @param data_len length of the data
/// @return 0 on success, 1 on failure
uint8_t COMMS_downlink_data(uint8_t *data, uint32_t data_len) {
    //TODO: For now, only send as much data as can fit in one packet, we could use/implement rdp to allow downlinking of arbitrary amounts of data.
    if (data_len + 4 > CSP_MAX_TX_SIZE) {return 1;}

    uint8_t packet[data_len + 4];
    perpend_csp_header(packet, data, data_len);

    return send_csp_packet_over_i2c(packet, data_len + 4);
}

