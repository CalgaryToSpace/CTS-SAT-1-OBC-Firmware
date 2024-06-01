
#ifndef __INCLUDE_GUARD__TELECOMMAND_DEFINITIONS_H
#define __INCLUDE_GUARD__TELECOMMAND_DEFINITIONS_H

#include <stdint.h>

typedef enum {
    TCMD_TelecommandChannel_DEBUG_UART,
    TCMD_TelecommandChannel_RADIO1
} TCMD_TelecommandChannel_enum_t;

typedef uint8_t (*TCMD_TCMDEXEC_Function_Ptr)(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                                         char *response_output_buf, uint16_t response_output_buf_len);

typedef struct {
	char* tcmd_name;
	TCMD_TCMDEXEC_Function_Ptr tcmd_func;
    uint8_t number_of_args;

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

uint8_t TCMDEXEC_upload_mpi_firmware_page(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);

#endif // __INCLUDE_GUARD__TELECOMMAND_DEFINITIONS_H
