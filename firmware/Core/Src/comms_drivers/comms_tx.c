#include "comms_drivers/comms_tx.h"
#include "memory.h"

uint8_t COMMS_downlink_tcmd_response(
    uint64_t ts_sent, 
    uint8_t respose_code, 
    uint16_t duration_ms, 
    char *respose,
    uint32_t respose_len
) {
    CTS_packet_t packet;
    packet.packet_type = COMMS_PACKET_TYPE_TCMD_RESPONSE;

    uint8_t header_len = sizeof(ts_sent) + sizeof(respose_code) + sizeof(duration_ms) + sizeof(respose_len);
    if( respose_len + header_len > AX100_DOWNLINK_MAX_BYTES) {return 1;}

    uint8_t packet_len = 0;
    memcpy(packet.data + packet_len, &ts_sent, sizeof(ts_sent)); // 8 bytes of ts_sent
    packet_len += sizeof(ts_sent);

    memcpy(packet.data + packet_len, &respose_code, sizeof(respose_code)); // 1 byte of respose_code
    packet_len += sizeof(respose_code);

    memcpy(packet.data + packet_len, &duration_ms, sizeof(duration_ms)); // 2 bytes of duration_ms
    packet_len += sizeof(duration_ms);

    memcpy(packet.data + packet_len, respose, sizeof(respose_len)); // 1 byte of respose
    packet_len += sizeof(respose_len);
    

    return AX100_downlink_bytes((uint8_t *)(&packet), packet_len);
}
