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

/// @brief Set the system time to the provided Unix epoch time in milliseconds
/// @param args_str
/// - Arg 0: Unix epoch time in milliseconds (uint64_t)
/// @return 0 if successful, 1 if error
uint8_t TCMDEXEC_set_system_time(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
  
    uint64_t ms = 0;

    uint8_t result = TCMD_extract_uint64_arg(args_str, strlen(args_str), 0, &ms);
    if (result != 0) {
        return 1;
    }
    TIM_set_current_unix_epoch_time_ms(ms, TIM_SOURCE_TELECOMMAND);
    snprintf(response_output_buf, response_output_buf_len, "Updated system time");
    return 0;
}

/// @brief Adjust the system time with a signed int
/// @param args_str
/// - Arg 0: Correction time in milliseconds (int64_t)
/// @return 0 if successful, 1 if error
uint8_t TCMDEXEC_correct_system_time(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len){

    int64_t correction_time_ms=0;

    if(TCMD_ascii_to_int64(args_str, strlen(args_str), &correction_time_ms)!=0){ // Converting args_str to signed int and checks if arguement is correct
        snprintf(response_output_buf, response_output_buf_len, "Incorrect argument entered. Please enter a signed integer number of milliseconds");
        return 1;
    }

    TIM_set_current_unix_epoch_time_ms(TIM_get_current_unix_epoch_time_ms()+correction_time_ms, TIM_SOURCE_TELECOMMAND);
    snprintf(response_output_buf, response_output_buf_len, "Updated system time");
    
    return 0;
}

