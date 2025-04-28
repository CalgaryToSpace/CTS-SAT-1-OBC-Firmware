#ifndef INCLUDE_GUARD__COMMS_TX_H__
#define INCLUDE_GUARD__COMMS_TX_H__

#include "comms_drivers/ax100_tx.h"

typedef enum {
    COMMS_PACKET_TYPE_BEACON_MINIMAL = 0x01,
    COMMS_PACKET_TYPE_BEACON_FULL = 0x02,
    COMMS_PACKET_TYPE_LOG_MESSAGE = 0x03,
    COMMS_PACKET_TYPE_TCMD_RESPONSE = 0x04,
    COMMS_PACKET_TYPE_DOWNLINK_FIRST_PACKET = 0x05,
    COMMS_PACKET_TYPE_DOWNLINK_NEXT_PACKET = 0x06,
    COMMS_PACKET_TYPE_DOWNLINK_LAST_PACKET = 0x07
} COMMS_packet_type_enum_t;

// (AX100_DOWNLINK_MAX_BYTES - 1-8-1-2-1-1)
#define COMMS_TCMD_RESPONSE_PACKET_MAX_DATA_BYTES_PER_PACKET 186

#pragma pack(push, 1)

typedef struct {
    uint8_t packet_type; // COMMS_packet_type_enum_t - Always COMMS_PACKET_TYPE_TCMD_RESPONSE for this packet

    uint64_t ts_sent;        // 8 bytes
    uint8_t response_code;   // 1 byte
    uint16_t duration_ms;    // 2 bytes
    uint8_t response_seq_num; // 1 byte
    uint8_t response_max_seq_num;   // 1 byte

    uint8_t data[COMMS_TCMD_RESPONSE_PACKET_MAX_DATA_BYTES_PER_PACKET];
} COMMS_tcmd_response_packet_t;

#pragma pack(pop)


uint8_t COMMS_downlink_tcmd_response(
    uint64_t ts_sent, 
    uint8_t response_code, 
    uint16_t duration_ms, 
    char *response,
    uint32_t response_len
);
#endif // INCLUDE_GUARD__COMMS_TX_H__