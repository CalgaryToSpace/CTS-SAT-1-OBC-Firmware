#include "crc.h"
#include "telecommands/telecommand_args_helpers.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include "telecommands/telecommand_types.h"

uint8_t TCMDEXEC_crc(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,char *response_output_buf,
                        uint16_t response_output_buf_len){
                            // say args_str = "01 02 03"
    const char *input[100]='\0';
    uint16_t crc_size;
    int64_t crc_result;
    char message[10];
    int64_t checksum = -137262718;

    const uint8_t parse_crc_result  = TCMD_extract_string_arg(args_str, 0, input, &crc_size);

    // input_arry = {0x01, 0x02, 0x03, garbage, garbage, garbage, x97}
    if(parse_crc_result==0)
    {
        snprintf(response_output_buf, response_output_buf_len, "Error: Error extracting hex argument.");
        return 2;
    }

    GEN_crc32_checksum(&hcrc1,input, crc_size, &crc_result);

    if(checksum == crc_result)
    {
        // message[0] = "match";
        strcpy(message, "match");

    }
    else
    {
        // message[0] = "conflict";
        strcpy(message, "conflict");
    }

    snprintf(response_output_buf, response_output_buf_len, "The crc result is %d.\n The checksums %s.\n", (int32_t*)crc_result, message);
    return 0;
}