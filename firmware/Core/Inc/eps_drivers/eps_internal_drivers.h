
#ifndef INCLUDE_GUARD__EPS_INTERNAL_DRIVERS_H__
#define INCLUDE_GUARD__EPS_INTERNAL_DRIVERS_H__

#include <stdint.h>

// #pragma region Constants

// Note: EPS_I2C_ADDR is left-shifted by 1 to be compatible with the HAL_I2C functions
// const uint8_t EPS_I2C_ADDR = (0x20 << 1); // EPS I2C address
// const uint8_t EPS_COMMAND_STID = 0x1A; // "System Type Identifier (STID)" (Software ICD, page 17)
// const uint8_t EPS_COMMAND_IVID = 0x07; // "Interface Version Identifier (IVID)" (Software ICD, page 18)
// const uint8_t EPS_COMMAND_BID = 0x01; // "Board Identifier (BID)" (Software ICD, page 20)
// const uint8_t EPS_DEFAULT_RX_LEN_MIN = 5; // for commands with no response params, 5 bytes are returned
// const uint8_t EPS_ENABLE_DEBUG_PRINT = 1; // bool; 0 to disable
// const uint32_t EPS_MAX_RESPONSE_POLL_TIME_MS = 100;

#define EPS_I2C_ADDR (0x20 << 1) // EPS I2C address

#define EPS_COMMAND_STID 0x1A // "System Type Identifier (STID)" (Software ICD, page 17)
#define EPS_COMMAND_IVID 0x07 // "Interface Version Identifier (IVID)" (Software ICD, page 18)
#define EPS_COMMAND_BID 0x00 // "Board Identifier (BID)" (Software ICD, page 20)

#define EPS_DEFAULT_RX_LEN_MIN 5 // for commands with no response params, 5 bytes are returned

#define EPS_ENABLE_DEBUG_PRINT 1 // bool; 0 to disable

#define EPS_MAX_RESPONSE_POLL_TIME_MS 100


// #pragma endregion Constants


// #pragma region Function_Prototypes

uint8_t EPS_send_cmd_get_response(const uint8_t cmd_buf[], uint8_t cmd_buf_len,
        uint8_t rx_buf[], uint16_t rx_buf_len);

uint8_t EPS_run_argumentless_cmd(uint8_t command_code);

// #pragma endregion Function_Prototypes

#endif /* INCLUDE_GUARD__EPS_INTERNAL_DRIVERS_H__ */
