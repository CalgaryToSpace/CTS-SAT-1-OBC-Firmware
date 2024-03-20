
#ifndef __INCLUDE_GUARD__TELECOMMAND_DEFINITIONS_H
#define __INCLUDE_GUARD__TELECOMMAND_DEFINITIONS_H

#include <stdint.h>
#include "telecommands/telecommand_definitions.h"

typedef enum {
    TCMD_TelecommandChannel_DEBUG_UART,
    TCMD_TelecommandChannel_RADIO1
} TCMD_TelecommandChannel_enum_t;

typedef struct {
	char* tcmd_name;
	void (*tcmd_func)(char*, TCMD_TelecommandChannel_enum_t, char*, uint16_t);
    uint8_t number_of_args;

    // TODO: maybe add other fields, including priority, expected execution duration, "config vs. action", etc.

} TCMD_TelecommandDefinition_t;



uint8_t TCMDEXEC_hello_world(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_core_system_stats(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);

uint8_t TCMDEXEC_echo_back_args(const uint8_t *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len);



#endif // __INCLUDE_GUARD__TELECOMMAND_DEFINITIONS_H
