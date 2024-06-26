#include "telecommands/timekeeping_telecommand_defs.h"
#include "telecommands/telecommand_args_helpers.h"

#include "timekeeping/timekeeping.h"

#include <stdio.h>
#include <string.h>

uint8_t TCMDEXEC_get_system_time(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    TIM_get_timestamp_string(response_output_buf, response_output_buf_len);
    return 0;
}

uint8_t TCMDEXEC_set_system_time(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
  
    uint64_t ms = 0;

    uint8_t result = TCMD_extract_uint64_arg((char*)args_str, strlen((char*)args_str), 0, &ms);
    if (result != 0) {
        return 1;
    }
    TIM_set_current_unix_epoch_time_ms(ms, TIM_SOURCE_TELECOMMAND);
    snprintf(response_output_buf, response_output_buf_len, "Updated system time");
    return 0;
}

