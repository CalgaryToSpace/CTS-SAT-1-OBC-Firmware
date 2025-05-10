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
        AX100_DOWNLINK_MAX_BYTES - COMMS_TCMD_RESPONSE_PACKET_MAX_DATA_BYTES_PER_PACKET
    );

    const uint8_t max_seq_num = (
        (response_len + COMMS_TCMD_RESPONSE_PACKET_MAX_DATA_BYTES_PER_PACKET - 1)
        / COMMS_TCMD_RESPONSE_PACKET_MAX_DATA_BYTES_PER_PACKET
    );

    uint16_t response_start_idx = 0;
    int32_t remaining_len = response_len;
    uint8_t response_seq_num = 1;
    while (remaining_len > 0) {
        const uint16_t this_packet_len = (remaining_len > COMMS_TCMD_RESPONSE_PACKET_MAX_DATA_BYTES_PER_PACKET)
            ? COMMS_TCMD_RESPONSE_PACKET_MAX_DATA_BYTES_PER_PACKET
            : remaining_len;

        packet.response_max_seq_num = max_seq_num;
        packet.response_seq_num = response_seq_num++;

        // Copy the data into the packet
        memcpy(packet.data, &response[response_start_idx], this_packet_len);
        remaining_len -= this_packet_len;
        response_start_idx += this_packet_len;

        const uint8_t success = AX100_downlink_bytes((uint8_t *)(&packet), this_packet_len + header_len);
        if (success != 0) {
            return success;
        }
    }

    return 0;
}
