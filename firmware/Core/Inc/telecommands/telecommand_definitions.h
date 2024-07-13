
#ifndef __INCLUDE_GUARD__TELECOMMAND_DEFINITIONS_H
#define __INCLUDE_GUARD__TELECOMMAND_DEFINITIONS_H

#include <stdint.h>

typedef enum {
    TCMD_TelecommandChannel_DEBUG_UART,
    TCMD_TelecommandChannel_RADIO1
} TCMD_TelecommandChannel_enum_t;

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

typedef uint8_t (*TCMD_TCMDEXEC_Function_Ptr)(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                         char *response_output_buf, uint16_t response_output_buf_len);

typedef struct {
	char* tcmd_name;
	TCMD_TCMDEXEC_Function_Ptr tcmd_func;
    uint8_t number_of_args;
    TCMD_readiness_level_enum_t readiness_level;
    // TODO: maybe add other fields, including priority, expected execution duration, "config vs. action", etc.

} TCMD_TelecommandDefinition_t;


extern const TCMD_TelecommandDefinition_t TCMD_telecommand_definitions[];
extern const int16_t TCMD_NUM_TELECOMMANDS;


uint8_t TCMDEXEC_hello_world(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_heartbeat_off(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_heartbeat_on(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_core_system_stats(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_echo_back_args(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_echo_back_uint32_args(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_run_all_unit_tests(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_available_telecommands(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);

#endif // __INCLUDE_GUARD__TELECOMMAND_DEFINITIONS_H
