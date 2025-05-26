#ifndef INCLUDE_GUARD__COMMS_TX_H__
#define INCLUDE_GUARD__COMMS_TX_H__

#include "comms_drivers/ax100_tx.h"

/// @brief Packet types for the COMMS downlink packets.
/// @details This is the first byte (after the CSP header) of the downlink packets.
typedef enum {
    COMMS_PACKET_TYPE_BEACON_MINIMAL = 0x01,
    COMMS_PACKET_TYPE_BEACON_FULL = 0x02,
    COMMS_PACKET_TYPE_LOG_MESSAGE = 0x03,
    COMMS_PACKET_TYPE_TCMD_RESPONSE = 0x04,
    COMMS_PACKET_TYPE_BULK_FILE_DOWNLINK = 0x10,
} COMMS_packet_type_enum_t;


// (AX100_DOWNLINK_MAX_BYTES - 1)
#define COMMS_LOG_MESSAGE_PACKET_MAX_DATA_BYTES_PER_PACKET 199

#if COMMS_LOG_MESSAGE_PACKET_MAX_DATA_BYTES_PER_PACKET != (AX100_DOWNLINK_MAX_BYTES - 1)
#error "COMMS_LOG_MESSAGE_PACKET_MAX_DATA_BYTES_PER_PACKET is incorrect"
#endif


// (AX100_DOWNLINK_MAX_BYTES - 1-8-1-2-1-1)
#define COMMS_TCMD_RESPONSE_PACKET_MAX_DATA_BYTES_PER_PACKET 186


#if COMMS_TCMD_RESPONSE_PACKET_MAX_DATA_BYTES_PER_PACKET != (AX100_DOWNLINK_MAX_BYTES - 1 - 8 - 1 - 2 - 1 - 1)
#error "COMMS_TCMD_RESPONSE_PACKET_MAX_DATA_BYTES_PER_PACKET is incorrect"
#endif

// (AX100_DOWNLINK_MAX_BYTES - 1-1-1-4)
#define COMMS_BULK_FILE_DOWNLINK_PACKET_MAX_DATA_BYTES_PER_PACKET 193

#if COMMS_BULK_FILE_DOWNLINK_PACKET_MAX_DATA_BYTES_PER_PACKET != (AX100_DOWNLINK_MAX_BYTES - 1 - 1 - 1 - 4)
#error "COMMS_BULK_FILE_DOWNLINK_PACKET_MAX_DATA_BYTES_PER_PACKET is incorrect"
#endif

#pragma pack(push, 1)

typedef struct {
    uint8_t packet_type; // COMMS_packet_type_enum_t - Always COMMS_PACKET_TYPE_LOG_MESSAGE for this packet

    uint8_t data[COMMS_LOG_MESSAGE_PACKET_MAX_DATA_BYTES_PER_PACKET];
} COMMS_log_message_packet_t;


typedef struct {
    uint8_t packet_type; // COMMS_packet_type_enum_t - Always COMMS_PACKET_TYPE_TCMD_RESPONSE for this packet

    uint64_t ts_sent;        // 8 bytes
    uint8_t response_code;   // 1 byte
    uint16_t duration_ms;    // 2 bytes
    uint8_t response_seq_num; // 1 byte
    uint8_t response_max_seq_num;   // 1 byte

    uint8_t data[COMMS_TCMD_RESPONSE_PACKET_MAX_DATA_BYTES_PER_PACKET];
} COMMS_tcmd_response_packet_t;

typedef struct {
    uint8_t packet_type; // COMMS_packet_type_enum_t - Always COMMS_PACKET_TYPE_BULK_FILE_DOWNLINK for this packet

    uint8_t file_seq_num;   // 1 byte
    uint8_t file_max_seq_num;   // 1 byte

    uint32_t file_offset;   // 4 bytes

    uint8_t data[COMMS_BULK_FILE_DOWNLINK_PACKET_MAX_DATA_BYTES_PER_PACKET];
} COMMS_bulk_file_downlink_packet_t;

// TODO: Add sizeof assertions in unit tests related to the packets above.
// assert(sizeof(COMMS_log_message_packet_t) == AX100_DOWNLINK_MAX_BYTES);
// assert(sizeof(COMMS_tcmd_response_packet_t) == AX100_DOWNLINK_MAX_BYTES);
// assert(sizeof(COMMS_bulk_file_downlink_packet_t) == AX100_DOWNLINK_MAX_BYTES);

#pragma pack(pop)


uint8_t COMMS_downlink_tcmd_response(
    uint64_t ts_sent, 
    uint8_t response_code, 
    uint16_t duration_ms, 
    char *response,
    uint32_t response_len
);

uint8_t COMMS_downlink_log_message(const char log_message_str[]);

uint8_t COMMS_downlink_bulk_file_downlink(
    uint8_t file_seq_num,
    uint8_t file_max_seq_num,
    uint32_t file_offset,
    uint8_t data[],
    uint16_t data_len
);

#endif // INCLUDE_GUARD__COMMS_TX_H__
