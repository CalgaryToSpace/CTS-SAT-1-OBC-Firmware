#include "comms_drivers/comms_tx.h"
#include "comms_drivers/beacon.h"
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
        const uint16_t this_data_len = (remaining_len > COMMS_TCMD_RESPONSE_PACKET_MAX_DATA_BYTES_PER_PACKET)
            ? COMMS_TCMD_RESPONSE_PACKET_MAX_DATA_BYTES_PER_PACKET
            : remaining_len;

        packet.response_max_seq_num = max_seq_num;
        packet.response_seq_num = response_seq_num++;

        // Copy the data into the packet
        memcpy(packet.data, &response[response_start_idx], this_data_len);
        remaining_len -= this_data_len;
        response_start_idx += this_data_len;

        const uint8_t success = AX100_downlink_bytes((uint8_t *)(&packet), header_len + this_data_len);
        if (success != 0) {
            return success;
        }
    }

    return 0;
}


// Note: This packet is allocated in the data segment (global) to avoid requiring lots of stack space
// in every task that emits a log. Could be stack-allocated, but would need to increase all stack sizes
// by about 256 bytes.
// Only used within `COMMS_downlink_log_message`.
static COMMS_log_message_packet_t log_msg_packet;

uint8_t COMMS_downlink_log_message(const char log_message_str[]) {
    // Safety: Clear the packet because we reuse it.
    memset(&log_msg_packet, 0, sizeof(log_msg_packet));

    log_msg_packet.packet_type = COMMS_PACKET_TYPE_LOG_MESSAGE;

    const uint8_t header_len = (
        AX100_DOWNLINK_MAX_BYTES - COMMS_LOG_MESSAGE_PACKET_MAX_DATA_BYTES_PER_PACKET
    );

    const uint16_t log_message_len = strlen(log_message_str);
    const uint16_t this_data_len = (log_message_len > COMMS_LOG_MESSAGE_PACKET_MAX_DATA_BYTES_PER_PACKET)
        ? COMMS_LOG_MESSAGE_PACKET_MAX_DATA_BYTES_PER_PACKET
        : log_message_len;

    // Copy the data into the packet.
    memcpy(log_msg_packet.data, log_message_str, this_data_len);

    const uint8_t success = AX100_downlink_bytes((uint8_t *)(&log_msg_packet), header_len + this_data_len);

    if (success != 0) {
        return success;
    }

    return 0;
}

uint8_t COMMS_downlink_bulk_file_downlink(
    uint8_t file_seq_num,
    uint8_t file_max_seq_num,
    uint32_t file_offset,
    uint8_t data[],
    uint16_t data_len
) {
    COMMS_bulk_file_downlink_packet_t packet;
    packet.packet_type = COMMS_PACKET_TYPE_BULK_FILE_DOWNLINK;
    packet.file_seq_num = file_seq_num;
    packet.file_max_seq_num = file_max_seq_num;
    packet.file_offset = file_offset;

    const uint8_t header_len = (
        AX100_DOWNLINK_MAX_BYTES - COMMS_BULK_FILE_DOWNLINK_PACKET_MAX_DATA_BYTES_PER_PACKET
    );

    const uint16_t this_data_len = (data_len > COMMS_BULK_FILE_DOWNLINK_PACKET_MAX_DATA_BYTES_PER_PACKET)
        ? COMMS_BULK_FILE_DOWNLINK_PACKET_MAX_DATA_BYTES_PER_PACKET
        : data_len;

    // Copy the data into the packet.
    memcpy(packet.data, data, this_data_len);

    const uint8_t success = AX100_downlink_bytes((uint8_t *)(&packet), header_len + this_data_len);

    if (success != 0) {
        return success;
    }

    return 0;
}

static COMMS_beacon_basic_packet_t beacon_basic_packet;

uint8_t COMMS_downlink_beacon_basic_packet() {
    // Pack the packet with the current system state.
    // This is a function that only reads from memory (and can't fail), so no error handling.
    COMMS_fill_beacon_basic_packet(&beacon_basic_packet);

    const uint8_t success = AX100_downlink_bytes(
        (uint8_t *)(&beacon_basic_packet), 
        sizeof(COMMS_beacon_basic_packet_t)
    );

    return success;
}
