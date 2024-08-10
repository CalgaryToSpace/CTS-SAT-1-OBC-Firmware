#include "telecommands/telecommand_definitions.h"
#include "telecommands/telecommand_args_helpers.h"
#include "telecommands/telecommand_executor.h"
#include "debug_tools/debug_uart.h"
#include "telecommands/agenda_telecommands_def.h"

#include <string.h>
#include <stdio.h>

/// @brief Telecommand: Fetch all active agendas
/// @param args_str No arguments needed
/// @param tcmd_channel The channel on which the telecommand was received, and on which the response should be sent
/// @param response_output_buf The buffer to write the response to
/// @param response_output_buf_len The maximum length of the response_output_buf (its size)
/// @return 0 on success
uint8_t TCMDEXEC_agenda_fetch(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
     
    TCMD_agenda_fetch();

    return 0;
}