#ifndef INCLUDE_GUARD__AX100_TX_H__
#define INCLUDE_GUARD__AX100_TX_H__

#include <stdint.h>

extern uint32_t AX100_enable_downlink_uart_logs;
extern uint32_t AX100_enable_downlink_inhibited_uart_logs;

/// @brief The maximum number of CTS bytes that can be sent to the AX100 in a single downlink packet.
/// @note This value does not include the CSP header (4 bytes), but DOES include any CTS-SAT-1 header bytes (e.g., sequence numbers).
#define AX100_DOWNLINK_MAX_BYTES 200 // FIXME: Confirm this value

#define AX100_CSP_HEADER_LENGTH_BYTES 4

uint8_t AX100_downlink_bytes(uint8_t *data, uint16_t data_len);

#endif // INCLUDE_GUARD__AX100_TX_H__