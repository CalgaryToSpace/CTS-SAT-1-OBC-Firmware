
#ifndef INCLUDE_GUARD__TELECOMMAND_TYPES_H
#define INCLUDE_GUARD__TELECOMMAND_TYPES_H

#include <stdint.h>

#define TCMD_MAX_RESP_FNAME_LEN 64

typedef enum {
    TCMD_READINESS_LEVEL_IDEA_PHASE, 
    TCMD_READINESS_LEVEL_NOT_IMPLEMENTED, 
    TCMD_READINESS_LEVEL_IN_PROGRESS, 

    // Especially for commands which only work over the umbilical UART 
    TCMD_READINESS_LEVEL_GROUND_USAGE_ONLY,

    // For commands like "test the flash memory", which corrupts the filesystem but is flight safe
    TCMD_READINESS_LEVEL_FLIGHT_TESTING,

    // For commands which are intended for normal operation in flight
    TCMD_READINESS_LEVEL_FOR_OPERATION
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
    char args_str_no_parens[255]; // TODO: consider changing this to a pointer, and storing the args somewhere else to save memory
    /// @brief The value of the `@tssent` field when the telecommand was received.
    uint64_t timestamp_sent;
    /// @brief The value of the `@tsexec` field when the telecommand was received.
    uint64_t timestamp_to_execute;
    /// @brief Name of file that response should be written to, empty string otherwise
    char resp_fname[TCMD_MAX_RESP_FNAME_LEN];
} TCMD_parsed_tcmd_to_execute_t;

#endif // INCLUDE_GUARD__TELECOMMAND_TYPES_H
