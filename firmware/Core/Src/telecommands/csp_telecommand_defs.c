#include "telecommands/csp_telecommand_defs.h"

#include "cts_csp_main.h"


uint8_t TCMDEXEC_csp_demo_1(
    const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    CSP_demo_1();
    
    return 0;
}
