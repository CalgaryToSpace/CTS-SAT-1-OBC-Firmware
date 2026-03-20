
#ifndef INCLUDE_GUARD__TELECOMMAND_TYPES_H
#define INCLUDE_GUARD__TELECOMMAND_TYPES_H

#include <stdint.h>

#define TCMD_MAX_RESP_FNAME_LEN 64

// Max len of `args_str_no_parens` in `TCMD_parsed_tcmd_to_execute_t`, including null terminator.
#define TCMD_ARGS_STR_NO_PARENS_SIZE 240

/// @brief Max full length of a telecommand, including prefix, args, suffix tags, null terminator, etc.
/// @note This is a superset/container of the `TCMD_ARGS_STR_NO_PARENS_SIZE` value.
static const uint16_t TCMD_MAX_FULL_LENGTH = 255;


/// @brief The permission/risk level of a telecommand.
typedef enum {
    // For commands which are intended for normal operation in flight.
    TCMD_READINESS_LEVEL_FOR_OPERATION = 0,

    // For commands which are generally safe to use in flight, but which shouldn't regularly be used,
    // and which require expert knowledge on the mechanism of operation.
    TCMD_READINESS_LEVEL_FOR_RECOVERY_OR_EXPERT = 10,

    // For commands like "test the flash memory", which corrupts the filesystem but is flight-safe.
    TCMD_READINESS_LEVEL_FLIGHT_TESTING = 20,

    // Especially for commands which only work over the umbilical UART.
    TCMD_READINESS_LEVEL_GROUND_USAGE_ONLY = 30,

    // For commands which are high risk and unsafe.
    TCMD_READINESS_LEVEL_HIGH_RISK_AND_UNSAFE = 40,
} TCMD_readiness_level_enum_t;

typedef uint8_t (*TCMD_TCMDEXEC_Function_Ptr)(const char *args_str,
                                         char *response_output_buf, uint16_t response_output_buf_len);

typedef struct {
	char* tcmd_name;
	TCMD_TCMDEXEC_Function_Ptr tcmd_func;
    uint8_t number_of_args;
    TCMD_readiness_level_enum_t readiness_level;
    // TODO: maybe add other fields, including priority, expected execution duration, "config vs. action", etc.

} TCMD_TelecommandDefinition_t;


typedef struct {
    /// @brief The index of the telecommand in the `TCMD_telecommand_definitions` array.
    uint8_t tcmd_idx;
    char args_str_no_parens[TCMD_ARGS_STR_NO_PARENS_SIZE]; // TODO: consider changing this to a pointer, and storing the args somewhere else to save memory
    /// @brief The value of the `@tssent` field when the telecommand was received.
    uint64_t timestamp_sent;
    /// @brief The value of the `@tsexec` field when the telecommand was received.
    uint64_t timestamp_to_execute;
    /// @brief Name of file that response should be written to, empty string otherwise
    char resp_fname[TCMD_MAX_RESP_FNAME_LEN];
} TCMD_parsed_tcmd_to_execute_t;

#endif // INCLUDE_GUARD__TELECOMMAND_TYPES_H
