#include "comms_drivers/comms_tx.h"
#include <string.h>

uint8_t COMMS_downlink_tcmd_response(
    uint64_t ts_sent, 
    uint8_t response_code, 
    uint16_t duration_ms, 
    char *response,
    uint32_t response_len
) {
    COMMS_tcmd_response_packet_t packet;
    packet.packet_type = COMMS_PACKET_TYPE_TCMD_RESPONSE;
    packet.ts_sent = ts_sent;
    packet.response_code = response_code;
    packet.duration_ms = duration_ms;

    const uint8_t header_len = (
        sizeof(packet.packet_type)
        + sizeof(ts_sent) + sizeof(response_code) + sizeof(duration_ms)
    );

    if (response_len > AX100_DOWNLINK_MAX_BYTES - header_len) {
        response_len = AX100_DOWNLINK_MAX_BYTES - header_len;
    }

    memcpy(packet.data, response, response_len);

    return AX100_downlink_bytes((uint8_t *)(&packet), response_len + header_len);
}
