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
} COMMS_packet_type_t;

typedef struct {
    COMMS_packet_type_t packet_type;
    uint8_t data[AX100_DOWNLINK_MAX_BYTES];
} CTS_packet_t;

uint8_t COMMS_downlink_tcmd_response(
    uint64_t ts_sent, 
    uint8_t respose_code, 
    uint16_t duration_ms, 
    char *respose,
    uint32_t respose_len
);
#endif // INCLUDE_GUARD__COMMS_TX_H__